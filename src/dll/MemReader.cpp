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

#include "MemReader.h"
#include "RebirthMemReader.h"
#include "AfterbirthMemReader.h"

MemReader *MemReader::instance_ = nullptr;
ModuleInfo MemReader::module_info_;

MemReader *MemReader::GetMemoryReader()
{
    if (instance_ == nullptr)
    {
        if (GetIsaacExpansion() == Expansion::kAfterbirth)
            instance_ = new AfterbirthMemReader();
        else
            instance_ = new RebirthMemReader();
    }

    return instance_;
}

void MemReader::Destroy()
{
    if (instance_ != nullptr)
        delete instance_;

    instance_ = nullptr;
}

Expansion MemReader::GetIsaacExpansion()
{
    std::vector<unsigned char> afterbirth_present = SearchMemForVal(AfterbirthCheck);
    if (afterbirth_present.size() > 0)
        return kAfterbirth;
    else
        return kRebirth;
}

ModuleInfo MemReader::GetModuleInfo()
{
    if (module_info_.module_size > 0)    // Have we already calculated the module information?
        return module_info_;

    // Get the base address of the Isaac module
    DWORD module_handle = (DWORD)GetModuleHandleW(WCHAR_ISAAC_MODULE_NAME);
    MEMORY_BASIC_INFORMATION isaac_mem = {0 };
    if (VirtualQuery((LPVOID)module_handle, &isaac_mem, sizeof(isaac_mem)) == 0)
        throw std::runtime_error("Unable to get memory information for Isaac.");

    IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER*)module_handle;
    IMAGE_NT_HEADERS *pe_header = (IMAGE_NT_HEADERS*)((DWORD)dos_header->e_lfanew + (DWORD) isaac_mem.AllocationBase);
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE || pe_header->Signature != IMAGE_NT_SIGNATURE)
        throw std::runtime_error("The Isaac memory being accessed is incorrect.");

    module_info_.module_address = (DWORD) isaac_mem.AllocationBase;
    module_info_.module_size = pe_header->OptionalHeader.SizeOfImage;

    std::stringstream ss;
    ss << "Isaac module address: 0x" << std::hex << module_info_.module_address;
    QUEUE_LOG(mhud2::Log::LOG_INFO, ss.str());
    ss.str(""); ss.clear();

    ss << "Isaac module size: " << std::hex << module_info_.module_size;
    QUEUE_LOG(mhud2::Log::LOG_INFO, ss.str());
    ss.str(""); ss.clear();

    return module_info_;
}

std::vector<unsigned char> MemReader::SearchMemForVal(MemSig mem_sig)
{
    ModuleInfo module_info = GetModuleInfo();
    return SearchMemForVal(module_info, mem_sig);
}

std::vector<unsigned char> MemReader::SearchMemForVal(ModuleInfo module_info, MemSig mem_sig)
{
    std::vector<unsigned char> val_bytes;
    int sig_len = strlen(mem_sig.search_mask);
    unsigned char* p_search = (unsigned char*)module_info.module_address;
    unsigned char* p_search_end = (unsigned char*)(module_info.module_address + module_info.module_size - sig_len);

    while (p_search <= p_search_end)
    {
        int matching_bytes = 0;
        for (int i = 0; i < sig_len; ++i)
        {
            if (mem_sig.search_mask[i] != '?' && mem_sig.search_mask[i] != 'v'
                && p_search[i] != mem_sig.signature[i])
                break;
            ++matching_bytes;
        }

        if (matching_bytes == sig_len)
        {
            // Found the signature, grab the return value bytes
            for (int i = 0; i < sig_len; ++i)
            {
                if (mem_sig.search_mask[i] == 'v')
                {
                    val_bytes.push_back(p_search[i]);
                }
            }
        }

        ++p_search;
    }

    return val_bytes;
}
