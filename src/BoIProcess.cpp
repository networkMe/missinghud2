// Copyright 2015 Trevor Meehl
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//         http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
//         limitations under the License.

#include "BoIProcess.h"

BoIProcess* BoIProcess::instance_ = nullptr;

BoIProcess* BoIProcess::GetInstance()
{
    if (instance_ == nullptr)
        instance_ = new BoIProcess();

    return instance_;
}

void BoIProcess::Close()
{
    if (instance_ != nullptr)
        delete instance_;

    instance_ = nullptr;
}

BoIProcess::BoIProcess()
{
}

BoIProcess::~BoIProcess()
{
    UnhookBoIProcess();
}

bool BoIProcess::HookBoIProcess()
{
    // Walk through windows processes and open BoI
    PROCESSENTRY32W proc_entry = { 0 };
    proc_entry.dwSize = sizeof(proc_entry);
    HANDLE proc_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (proc_snapshot == INVALID_HANDLE_VALUE)
        throw std::runtime_error("[HookBoIProcess] Unable to obtain the system's process list in order to inject into Isaac.");
    if (!Process32FirstW(proc_snapshot, &proc_entry))
        throw std::runtime_error("[HookBoIProcess] Unable to read the system's process list in order to inject into Isaac.");
    do
    {
        if (std::wstring(proc_entry.szExeFile) == WCHAR_BOI_PROCESS_NAME)
        {
            process_id_ = proc_entry.th32ProcessID;
            process_ = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id_);
            if (process_ == NULL)
            {
                // Let's try with SeDebugPrivilege enabled
                if (!EnableDebugPrivilege())
                    throw std::runtime_error("[HookBoIProcess] Couldn't access the BoI process, then tried to enable "
                                             "debug privileges, that also failed! Try running MHUD2 as admin.");

                process_ = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id_);
                if (process_ == NULL)
                    throw std::runtime_error("[HookBoIProcess] Couldn't access the BoI process, even with debug privileges. "
                                             "Your anti-virus is probably blocking MHUD2.");
            }

            break;
        }
    } while (Process32NextW(proc_snapshot, &proc_entry));
    CloseHandle(proc_snapshot);

    if (process_id_ == 0)
        throw std::runtime_error("[HookBoIProcess] Unable to find an active Binding of Isaac process.");

    // Attempt to make sure that Isaac is initialized before we try to inject MHUD2
    WaitForBoIProcessInit();

    // Commit memory into the process for our DLL path
    LPVOID remote_mem = VirtualAllocEx(process_, 0, MAX_PATH * sizeof(wchar_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (remote_mem == NULL)
        throw std::runtime_error("[HookBoIProcess] Unable to commit memory into the active BoI process.");

    // Load in our hook DLL (MissingHUD2Hook.dll)
    LoadLibraryIntoBoI(remote_mem, "MissingHUD2Hook.dll");

    // Call "Start" method in the MissingHUD2Hook.dll
    FARPROC start_addr = GetRemoteProcAddress(injected_dll_, "MissingHUD2Hook.dll", "MHUD2_Start");
    HANDLE rThread = CreateRemoteThread(process_, NULL, 0, (LPTHREAD_START_ROUTINE)start_addr, NULL, 0, NULL);
    if (rThread == NULL)
        throw std::runtime_error("[HookBoIProcess] Couldn't execute MHUD2_Start in MissingHUD2Hook.dll.");
    CloseHandle(rThread);

    // Clear up the memory we committed
    VirtualFreeEx(process_, remote_mem, MAX_PATH * sizeof(wchar_t), MEM_RELEASE);
    return true;
}

bool BoIProcess::EnableDebugPrivilege()
{
    TOKEN_PRIVILEGES token_privs = { 0 };
    token_privs.PrivilegeCount = 1;
    HANDLE proc_token = NULL;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &proc_token) ||
        !LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &token_privs.Privileges[0].Luid))
        return false;

    token_privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    bool adjust_result = (bool)AdjustTokenPrivileges(proc_token, FALSE, &token_privs, 0, NULL, NULL);

    CloseHandle(proc_token);
    return adjust_result;
}

HMODULE BoIProcess::LoadLibraryIntoBoI(LPVOID remote_mem, std::string library)
{
    // Get path of library to inject
    FARPROC load_library_addr = GetRemoteProcAddress(GetModuleHandleW(L"kernel32.dll"), "kernel32.dll", "LoadLibraryW");
    std::wstring dir_path = QCoreApplication::applicationDirPath().toStdWString();
    std::wstring library_path = dir_path + L"/" + QString::fromStdString(library).toStdWString();
    if (library_path.length() > MAX_PATH)
        throw std::runtime_error("[LoadLibraryIntoBoI] Path of " + library + " is too long..." +
                                         QString::fromStdWString(library_path).toStdString());

    // Run LoadLibraryW in the remote process (BoI)
    LOG(INFO) << "Injecting dll: " << QString::fromStdWString(library_path).toStdString();
    if (WriteProcessMemory(process_, remote_mem, library_path.c_str(), library_path.length() * sizeof(wchar_t), NULL) == 0)
        throw std::runtime_error("[LoadLibraryIntoBoI] Couldn't write the path to " + library + " into the BoI process.");
    HANDLE rem_thread = CreateRemoteThread(process_, NULL, 0, (LPTHREAD_START_ROUTINE)load_library_addr, remote_mem, 0, NULL);
    if (rem_thread == NULL)
        throw std::runtime_error("[LoadLibraryIntoBoI] Couldn't execute LoadLibraryW in the BoI process.");
    WaitForSingleObject(rem_thread, INFINITE);

    // Make sure the DLL loaded properly
    DWORD exit_code = 0;
    GetExitCodeThread(rem_thread, &exit_code);
    if (exit_code == 0)
        throw std::runtime_error("[LoadLibraryIntoBoI] Error while executing LoadLibraryW in the BoI process. "
                                 "Try running Missing HUD 2 from another directory path.");
    CloseHandle(rem_thread);

    // Record the LoadLibraryW return code so that we can FreeLibrary when it comes time
    injected_dll_ = (HMODULE)exit_code;

    // Cleanup what we wrote to remote memory
    wchar_t zero_data[MAX_PATH] = { 0 };
    WriteProcessMemory(process_, remote_mem, zero_data, sizeof(zero_data), NULL);

    LOG(INFO) << "Injected " << library << " into BoI process with handle 0x" << std::hex << exit_code << ".";
    return (HMODULE)exit_code;
}

bool BoIProcess::IsRunning()
{
    DWORD exit_code = 0;
    if (GetExitCodeProcess(process_, &exit_code) == 0)
        return false;

    return (exit_code == STILL_ACTIVE);
}

bool BoIProcess::MHUD2Active()
{
    bool mhud2_active = false;

    // Check the modules associated with the Rebirth process for MissingHUD2Hook.dll
    MODULEENTRY32W module_entry = { 0 };
    module_entry.dwSize = sizeof(module_entry);
    HANDLE module_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id_);
    if (module_snapshot == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Unable to take a snapshot of Rebirth's active modules.");
    bool module32first_result = (bool)Module32FirstW(module_snapshot, &module_entry);
    while (!module32first_result && GetLastError() == ERROR_BAD_LENGTH)  // MSDN says keep trying until non-failure if ERROR_BAD_LENGTH
    {
        module32first_result = (bool)Module32FirstW(module_snapshot, &module_entry);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (!module32first_result)
        throw std::runtime_error("Unable to read the snapshot of Rebirth's active modules.");
    do
    {
        if (std::wstring(module_entry.szModule).find(L"MissingHUD2Hook.dll") != std::string::npos)
        {
            mhud2_active = true;
            break;
        }
    } while (Module32NextW(module_snapshot, &module_entry));
    CloseHandle(module_snapshot);

    return mhud2_active;
}

bool BoIProcess::UnhookBoIProcess()
{
    if (process_ == NULL)
        return true;

    // If the process is still running we should unhook our dll
    if (IsRunning() && MHUD2Active())
    {
        LOG(INFO) << "Unhooking MHUD2 from the active BoI process.";

        // Call the "MHUD2_Stop" function on the injected DLL, it should clean everything up inside Isaac by itself
        FARPROC stop_addr = GetRemoteProcAddress(injected_dll_, "MissingHUD2Hook.dll", "MHUD2_Stop");
        HANDLE rem_thread = CreateRemoteThread(process_, NULL, 0, (LPTHREAD_START_ROUTINE)stop_addr, NULL, 0, NULL);
        if (rem_thread == NULL)
            throw std::runtime_error("[HookBoIProcess] Couldn't execute MHUD2_Stop in MissingHUD2Hook.dll.");
        WaitForSingleObject(rem_thread, 5000);
        CloseHandle(rem_thread);
    }

    // Close the process handle
    return (bool)CloseHandle(process_);
}

FARPROC BoIProcess::GetRemoteProcAddress(HMODULE rem_dll_module, std::string rem_module_name, std::string proc_name)
{
    // Load the DLL into the local process (if it's not already loaded)
    std::wstring win_module_name = QString::fromStdString(rem_module_name).toStdWString();
    HMODULE local_hud2hookdll = GetModuleHandleW(win_module_name.c_str());
    if (local_hud2hookdll == NULL)
    {
        std::wstring hud2hookdll_path = QCoreApplication::applicationDirPath().toStdWString() + L"/" + win_module_name;
        local_hud2hookdll = LoadLibraryW(hud2hookdll_path.c_str());
        if (local_hud2hookdll == NULL)
            throw std::runtime_error("[GetRemoteProcAddress] Couldn't load " + rem_module_name + " into local process.");
    }

    // Get the local address of the procedure and calculate the offset
    FARPROC local_address = GetProcAddress(local_hud2hookdll, proc_name.c_str());
    if (local_address == NULL)
        throw std::runtime_error("[GetRemoteProcAddress] Unable to GetProcAddress in " + rem_module_name);
    DWORD proc_offset = (DWORD)local_address - (DWORD)local_hud2hookdll;

    FARPROC remote_address = (FARPROC)((DWORD)rem_dll_module + (DWORD)proc_offset);
    LOG(INFO) << "Local " << proc_name << " address: " << std::hex << (DWORD)local_address;
    LOG(INFO) << "Remote " << proc_name << " address: " << std::hex << (DWORD)remote_address;

    // Return the remote address equivalent of the function
    return remote_address;
}

void BoIProcess::WaitForBoIProcessInit()
{
    if (process_ == NULL)
        return;

    // Make sure the BoI process has been executing for at least 5 seconds, or wait for it to have been
    FILETIME process_timing[4] = { 0 };
    if (GetProcessTimes(process_, &process_timing[0], &process_timing[1], &process_timing[2], &process_timing[3]) == 0)
        throw std::runtime_error("[HookBoIProcess] Unable to GetProcessTimes for the active BoI executable.");
    ULARGE_INTEGER process_created_lg = { 0 };
    process_created_lg.LowPart = process_timing[0].dwLowDateTime;
    process_created_lg.HighPart = process_timing[0].dwHighDateTime;
    uint64_t process_created = process_created_lg.QuadPart;

    FILETIME system_timing = { 0 };
    GetSystemTimeAsFileTime(&system_timing);
    ULARGE_INTEGER system_time_lg = { 0 };
    system_time_lg.LowPart = system_timing.dwLowDateTime;
    system_time_lg.HighPart = system_timing.dwHighDateTime;
    uint64_t system_time = system_time_lg.QuadPart;

    if (process_created > (system_time - 50000000)) // 5 seconds
    {
        auto nanoseconds_to_sleep = std::chrono::nanoseconds((process_created - (system_time - 50000000)) * 100);
        LOG(INFO) << "Waiting " << std::chrono::duration_cast<std::chrono::milliseconds>(nanoseconds_to_sleep).count()
            << " milliseconds before injecting.";
        std::this_thread::sleep_for(nanoseconds_to_sleep);
    }
}
