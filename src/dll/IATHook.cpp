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

bool IATHook::InitIATHook(HMODULE app_module, std::string target_module, std::string proc_name, LPVOID detour_proc)
{
    // Get necessary PE information
    IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER*)app_module;

    MEMORY_BASIC_INFORMATION module_mem = { 0 };
    if (!VirtualQuery((LPVOID)app_module, &module_mem, sizeof(module_mem)))
        return false;

    IMAGE_NT_HEADERS *pe_header = (IMAGE_NT_HEADERS*)((DWORD)dos_header->e_lfanew + (DWORD)module_mem.AllocationBase);
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE || pe_header->Signature != IMAGE_NT_SIGNATURE)
        return false;

    IMAGE_IMPORT_DESCRIPTOR *import_descriptor = (IMAGE_IMPORT_DESCRIPTOR *)((DWORD)pe_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress + (DWORD)module_mem.AllocationBase);
    if (!(import_descriptor->Name))
        return false;

    while (import_descriptor->Name)
    {
        const char* imported_dll_name = (const char*)((DWORD)import_descriptor->Name + (DWORD)module_mem.AllocationBase);
        OutputDebugString(imported_dll_name);

        if (std::string(imported_dll_name) == target_module)
        {
            IMAGE_THUNK_DATA *thunk = (IMAGE_THUNK_DATA*)((DWORD)import_descriptor->FirstThunk + (DWORD)module_mem.AllocationBase);
            IMAGE_THUNK_DATA *orig_thunk = (IMAGE_THUNK_DATA*)((DWORD)import_descriptor->OriginalFirstThunk + (DWORD)module_mem.AllocationBase);

            for(; orig_thunk->u1.Function != 0; ++orig_thunk, ++thunk)
            {
                if (orig_thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) // ignore ordinal imports
                    continue;

                IMAGE_IMPORT_BY_NAME *import_via_name = (IMAGE_IMPORT_BY_NAME*)((DWORD)orig_thunk->u1.AddressOfData + (DWORD)module_mem.AllocationBase);
                const char* import_proc_name = (const char*)(import_via_name->Name);
                if (std::string(import_proc_name) == proc_name)
                {
                    // Found the proc to hook
                    IATHookInfo iat_hook_info;
                    iat_hook_info.app_module = app_module;
                    iat_hook_info.target_module = target_module;
                    iat_hook_info.proc_name = proc_name;
                    iat_hook_info.detour_proc = detour_proc;
                    iat_hook_info.orig_proc_addr = (LPVOID)thunk->u1.Function;
                    iat_hook_info.thunk = thunk;
                    iat_hooks_.insert(std::make_pair(proc_name, iat_hook_info));

                    return true;
                }
            }

            // We didn't find the proc to hook
            return false;
        }

        ++import_descriptor;
    }

    // We didn't find the DLL to hook :(
    return false;
}

bool IATHook::EnableIATHook(std::string proc_name, LPVOID *orig_proc_addr)
{
    if (iat_hooks_.count(proc_name) == 0)
        return false;

    IATHookInfo iathook_info = iat_hooks_[proc_name];

    // Make the import memory page writable
    MEMORY_BASIC_INFORMATION thunk_mem_info;
    if (!VirtualQuery(iathook_info.thunk, &thunk_mem_info, sizeof(MEMORY_BASIC_INFORMATION)))
        return false;
    if (!VirtualProtect(thunk_mem_info.BaseAddress, thunk_mem_info.RegionSize, PAGE_READWRITE, &thunk_mem_info.Protect))
        return false;

    // Replace the IAT function pointer
    *(orig_proc_addr) = iathook_info.orig_proc_addr;
    iathook_info.thunk->u1.Function = (DWORD)iathook_info.detour_proc;

    // Restore the import memory page permissions
    DWORD dummy;
    VirtualProtect(thunk_mem_info.BaseAddress, thunk_mem_info.RegionSize, thunk_mem_info.Protect, &dummy);

    return true;
}

bool IATHook::DisableIATHook(std::string proc_name)
{
    if (iat_hooks_.count(proc_name) == 0)
        return false;

    IATHookInfo iathook_info = iat_hooks_[proc_name];

    // Make the import memory page writable
    MEMORY_BASIC_INFORMATION thunk_mem_info;
    if (!VirtualQuery(iathook_info.thunk, &thunk_mem_info, sizeof(MEMORY_BASIC_INFORMATION)))
        return false;
    if (!VirtualProtect(thunk_mem_info.BaseAddress, thunk_mem_info.RegionSize, PAGE_READWRITE, &thunk_mem_info.Protect))
        return false;

    // Replace the IAT function pointer with the old function
    iathook_info.thunk->u1.Function = (DWORD)iathook_info.orig_proc_addr;

    // Restore the import memory page permissions
    DWORD dummy;
    VirtualProtect(thunk_mem_info.BaseAddress, thunk_mem_info.RegionSize, thunk_mem_info.Protect, &dummy);

    return true;
}
