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

#include "IATHook.h"

std::map<std::string, IATHookInfo> IATHook::iat_hooks_;

void IATHook::InitIATHook(HMODULE app_module, std::string target_module_name, std::string proc_name, LPVOID detour_proc)
{
    // Get necessary PE information
    IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER*)app_module;

    MEMORY_BASIC_INFORMATION module_mem = { 0 };
    if (!VirtualQuery((LPVOID)app_module, &module_mem, sizeof(module_mem)))
        throw std::runtime_error("Unable to retrieve memory information for app_module.");

    IMAGE_NT_HEADERS *pe_header = (IMAGE_NT_HEADERS*)((DWORD)dos_header->e_lfanew + (DWORD)module_mem.AllocationBase);
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE || pe_header->Signature != IMAGE_NT_SIGNATURE)
        throw std::runtime_error("The memory being read in app_module is incorrect.");

    IMAGE_IMPORT_DESCRIPTOR *import_descriptor = (IMAGE_IMPORT_DESCRIPTOR*)((DWORD)pe_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress + (DWORD)module_mem.AllocationBase);
    if (!(import_descriptor->Name))
        throw std::runtime_error("The IMAGE_IMPORT_DESCRIPTOR structure is invalid in the app_module.");

    std::transform(target_module_name.begin(), target_module_name.end(), target_module_name.begin(), tolower);
    while (import_descriptor->Name)
    {
        const char* imported_dll_name = (const char*)((DWORD)import_descriptor->Name + (DWORD)module_mem.AllocationBase);
        std::string imported_dll_str = std::string(imported_dll_name);
        std::transform(imported_dll_str.begin(), imported_dll_str.end(), imported_dll_str.begin(), tolower);
        if (imported_dll_str == target_module_name)
        {
            IMAGE_THUNK_DATA *thunk = (IMAGE_THUNK_DATA*)((DWORD)import_descriptor->FirstThunk + (DWORD)module_mem.AllocationBase);
            IMAGE_THUNK_DATA *orig_thunk = (IMAGE_THUNK_DATA*)((DWORD)import_descriptor->OriginalFirstThunk + (DWORD)module_mem.AllocationBase);

            for(; orig_thunk->u1.Function != 0; ++orig_thunk, ++thunk)
            {
                IMAGE_IMPORT_BY_NAME *import_via_name = (IMAGE_IMPORT_BY_NAME*)((DWORD)orig_thunk->u1.AddressOfData + (DWORD)module_mem.AllocationBase);
                const char* import_proc_name = (const char*)(import_via_name->Name);
                if (std::string(import_proc_name) == proc_name)
                {
                    // Found the proc to hook
                    IATHookInfo iat_hook_info;
                    iat_hook_info.app_module = app_module;
                    iat_hook_info.target_module_name = target_module_name;
                    iat_hook_info.proc_name = proc_name;
                    iat_hook_info.detour_proc = detour_proc;
                    iat_hook_info.orig_proc_addr = (LPVOID)thunk->u1.Function;
                    iat_hook_info.thunk = thunk;
                    iat_hooks_.insert(std::make_pair(proc_name, iat_hook_info));

                    return;
                }
            }

            // We didn't find the proc to hook
            throw std::runtime_error("Procedure " + proc_name + " could not be found in " + target_module_name);
        }

        ++import_descriptor;
    }

    // We didn't find the DLL to hook :(
    throw std::runtime_error("Unable to find " + target_module_name + " in the IAT.");
}

void IATHook::EnableIATHook(std::string proc_name, LPVOID *orig_proc_addr)
{
    if (iat_hooks_.count(proc_name) == 0)
        throw std::runtime_error("The IAT hook for " + proc_name + " has not been initialized and cannot be enabled.");

    IATHookInfo iathook_info = iat_hooks_[proc_name];

    // Make the import memory page writable
    MEMORY_BASIC_INFORMATION thunk_mem_info;
    if (!VirtualQuery(iathook_info.thunk, &thunk_mem_info, sizeof(MEMORY_BASIC_INFORMATION)))
        throw std::runtime_error("Could not query thunk_mem_info when enabling IAT hook for " + proc_name);
    if (!VirtualProtect(thunk_mem_info.BaseAddress, thunk_mem_info.RegionSize, PAGE_READWRITE, &thunk_mem_info.Protect))
        throw std::runtime_error("Error occurred setting the IAT to have read/write privileges.");

    std::stringstream ss;
    ss << "Enabling IAT hook for " << proc_name << " in module " << iathook_info.target_module_name;
    QUEUE_LOG(mhud2::Log::LOG_INFO, ss.str());

    // Replace the IAT function pointer
    *(orig_proc_addr) = iathook_info.orig_proc_addr;
    iathook_info.thunk->u1.Function = (DWORD)iathook_info.detour_proc;

    // Restore the import memory page permissions
    DWORD dummy;
    VirtualProtect(thunk_mem_info.BaseAddress, thunk_mem_info.RegionSize, thunk_mem_info.Protect, &dummy);
}

void IATHook::DisableIATHook(std::string proc_name)
{
    if (iat_hooks_.count(proc_name) == 0)
        throw std::runtime_error("The IAT hook for " + proc_name + " has not been initialized and cannot be disabled.");

    IATHookInfo iathook_info = iat_hooks_[proc_name];

    // Make the import memory page writable
    MEMORY_BASIC_INFORMATION thunk_mem_info;
    if (!VirtualQuery(iathook_info.thunk, &thunk_mem_info, sizeof(MEMORY_BASIC_INFORMATION)))
        throw std::runtime_error("Could not query thunk_mem_info when disabling IAT hook for " + proc_name);
    if (!VirtualProtect(thunk_mem_info.BaseAddress, thunk_mem_info.RegionSize, PAGE_READWRITE, &thunk_mem_info.Protect))
        throw std::runtime_error("Error occurred setting the IAT to have read/write privileges.");

    std::stringstream ss;
    ss << "Disabling IAT hook for " << proc_name << " in module " << iathook_info.target_module_name;
    QUEUE_LOG(mhud2::Log::LOG_INFO, ss.str());

    // Replace the IAT function pointer with the old function
    iathook_info.thunk->u1.Function = (DWORD)iathook_info.orig_proc_addr;

    // Restore the import memory page permissions
    DWORD dummy;
    VirtualProtect(thunk_mem_info.BaseAddress, thunk_mem_info.RegionSize, thunk_mem_info.Protect, &dummy);
}
