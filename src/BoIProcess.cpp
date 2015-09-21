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
    PROCESSENTRY32 proc_entry = { 0 };
    proc_entry.dwSize = sizeof(proc_entry);
    HANDLE proc_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(proc_snapshot, &proc_entry)) {
        do {
            if (std::string(proc_entry.szExeFile) == BOI_PROCESS_NAME) {
                process_ = OpenProcess(PROCESS_ALL_ACCESS, FALSE, proc_entry.th32ProcessID);
                if (process_ == NULL)
                    throw std::runtime_error("[HookBoIProcess] Unable to open the running Binding of Isacc process.");
                break;
            }
        } while (Process32Next(proc_snapshot, &proc_entry));
    }
    CloseHandle(proc_snapshot);

    if (process_ == NULL)
        throw std::runtime_error("[HookBoIProcess] Unable to open an active Binding of Isaac process.");

    // Commit memory into the process for our DLL path
    LPVOID remote_mem = VirtualAllocEx(process_, 0, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (remote_mem == NULL)
        throw std::runtime_error("[HookBoIProcess] Unable to commit memory into the active BoI process.");

    // Load in our hook DLL (MissingHUD2Hook.dll)
    LoadLibraryIntoBoI(remote_mem, "MissingHUD2Hook.dll");

    // Call "Start" method in the MissingHUD2Hook.dll
    FARPROC start_addr = GetRemoteProcAddress(injected_dll_, "MissingHUD2Hook.dll", "HUD2_Start");
    HANDLE rThread = CreateRemoteThread(process_, NULL, 0, (LPTHREAD_START_ROUTINE)start_addr, NULL, 0, NULL);
    if (rThread == NULL)
        throw std::runtime_error("[HookBoIProcess] Couldn't execute Start in our MissingHUD2Hook.dll.");
    CloseHandle(rThread);

    // Clear up the memory we committed
    VirtualFreeEx(process_, remote_mem, MAX_PATH, MEM_RELEASE);
    return true;
}

HMODULE BoIProcess::LoadLibraryIntoBoI(LPVOID remote_mem, std::string library)
{
    // Get path of library to inject
    FARPROC load_library_addr = GetRemoteProcAddress(GetModuleHandle("kernel32.dll"), "kernel32.dll", "LoadLibraryA");
    char zero_data[MAX_PATH] = { 0 };
    std::string dir_path = QCoreApplication::applicationDirPath().toStdString();
    std::string library_path = dir_path + "/" + library;
    if (library_path.length() > MAX_PATH)
        throw std::runtime_error("[LoadLibraryIntoBoI] Path of " + library + " is too long.");

    // Run LoadLibraryA in the remote process (BoI)
    WriteProcessMemory(process_, remote_mem, library_path.c_str(), library_path.length(), NULL);
    HANDLE rem_thread = CreateRemoteThread(process_, NULL, 0, (LPTHREAD_START_ROUTINE)load_library_addr, remote_mem, 0, NULL);
    if (rem_thread == NULL)
        throw std::runtime_error("[LoadLibraryIntoBoI] Couldn't execute LoadLibraryA in the BoI process.");
    WaitForSingleObject(rem_thread, INFINITE);

    // Make sure the DLL loaded properly
    DWORD exit_code = 0;
    GetExitCodeThread(rem_thread, &exit_code);
    if (exit_code == 0)
        throw std::runtime_error("[LoadLibraryIntoBoI] Error while executing LoadLibraryA in the BoI process.");
    CloseHandle(rem_thread);

    // Record the LoadLibraryA return code so that we can FreeLibrary when it comes time
    injected_dll_ = (HMODULE)exit_code;

    // Cleanup what we wrote to remote memory
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

bool BoIProcess::UnhookBoIProcess()
{
    if (process_ == NULL)
        throw std::runtime_error("[UnhookBoIProcess] Process handle doesn't exist! We can't possibly be hooked in.");

    // If the process is still running we should unhook our dll
    if (IsRunning())
    {
        // Call the "Stop" function on the injected DLL, it should clean everything up inside Isaac by itself
        FARPROC stop_addr = GetRemoteProcAddress(injected_dll_, "MissingHUD2Hook.dll", "HUD2_Stop");
        HANDLE rem_thread = CreateRemoteThread(process_, NULL, 0, (LPTHREAD_START_ROUTINE)stop_addr, NULL, 0, NULL);
        if (rem_thread == NULL)
            throw std::runtime_error("[HookBoIProcess] Couldn't execute Start in our MissingHUD2Hook.dll.");
        CloseHandle(rem_thread);
    }

    // Close the process handle
    return (bool)CloseHandle(process_);
}

FARPROC BoIProcess::GetRemoteProcAddress(HMODULE rem_dll_module, std::string rem_module_name, std::string proc_name)
{
    // Load the DLL into the local process (if it's not already loaded)
    HMODULE local_hud2hookdll = GetModuleHandle(rem_module_name.c_str());
    if (local_hud2hookdll == NULL)
    {
        std::string hud2hookdll_path = QCoreApplication::applicationDirPath().toStdString() + "/" + rem_module_name;
        local_hud2hookdll = LoadLibrary(hud2hookdll_path.c_str());
        if (local_hud2hookdll == NULL)
            throw std::runtime_error("[GetRemoteProcAddress] Couldn't load " + rem_module_name + " into local process.");
    }

    // Get the local address of the procedure and calculate the offset
    FARPROC local_address = GetProcAddress(local_hud2hookdll, proc_name.c_str());
    if (local_address == NULL)
        throw std::runtime_error("[GetRemoteProcAddress] Unable to GetProcAddress of " + proc_name + " in " + rem_module_name);
    DWORD proc_offset = (DWORD)local_address - (DWORD)local_hud2hookdll;

    // Return the remote address equivalent of the function
    return (FARPROC)((DWORD)rem_dll_module + (DWORD)proc_offset);
}