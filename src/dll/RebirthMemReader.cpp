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

#include "RebirthMemReader.h"

RebirthMemReader *RebirthMemReader::mem_reader_ = nullptr;

RebirthMemReader *RebirthMemReader::GetMemoryReader()
{
    if (mem_reader_ == nullptr)
        mem_reader_ = new RebirthMemReader();

    return mem_reader_;
}

void RebirthMemReader::Destroy()
{
    if (mem_reader_)
        delete mem_reader_;

    mem_reader_ = nullptr;
}

RebirthMemReader::RebirthMemReader()
{
    GetRebirthModuleInfo();
}

RebirthMemReader::~RebirthMemReader()
{

}

bool RebirthMemReader::IsRunActive()
{
    // To check wether a run is currently active I check how many players there are
    // (just like Rebirth actually does)
    // When there are 0 players, then we are not in a run
    DWORD player_list = GetPlayerListMemAddr();
    if (player_list == 0)
        return false;

    int num_players = (int)((*(DWORD*)(player_list + sizeof(DWORD))) - (*(DWORD*) player_list));
    return (num_players > 0);
}

float RebirthMemReader::GetPlayerStatf(RebirthPlayerStat player_stat)
{
    DWORD player_class = GetPlayerMemAddr();
    if (player_class == 0)
        return 0.0f;

    float stat_val = *((float*)(player_class + player_stat));

    if (player_stat == RebirthPlayerStat::kRange)
    {
        stat_val *= -1; // Range is stored as a negative, but we show it positive on the HUD
    }

    return stat_val;
}

int RebirthMemReader::GetPlayerStati(RebirthPlayerStat player_stat)
{
    DWORD player_class = GetPlayerMemAddr();
    if (player_class == 0)
        return 0;

    int stat_val = *((int*)(player_class + player_stat));
    return stat_val;
}

DWORD RebirthMemReader::GetPlayerManagerMemAddr()
{
    DWORD player_manager_inst = *((DWORD*)(player_manager_inst_p_addr_));
    if (player_manager_inst == 0)
        return 0; // Player manager hasn't been initialized by Rebirth yet
    else
        return player_manager_inst;
}


DWORD RebirthMemReader::GetPlayerListMemAddr()
{
    DWORD player_manager_inst = GetPlayerManagerMemAddr();
    if (player_manager_inst == 0)
        return 0;

    if (player_manager_player_list_offset_ == 0)
        return 0;

    return (player_manager_inst + player_manager_player_list_offset_);
}

DWORD RebirthMemReader::GetPlayerMemAddr()
{
    // Here we follow the Rebirth memory address chain to get
    // the current player class associated with the current run
    DWORD player_list = GetPlayerListMemAddr();
    if (player_list == 0)
        return 0;

    DWORD player_p = *((DWORD*)player_list);
    return *((DWORD*)player_p);
}

void RebirthMemReader::GetRebirthModuleInfo()
{
    // Get the base address of the Rebirth module
    DWORD module_handle = (DWORD)GetModuleHandle(ISAAC_MODULE_NAME);
    MEMORY_BASIC_INFORMATION rebirth_mem = { 0 };
    if (VirtualQuery((LPVOID)module_handle, &rebirth_mem, sizeof(rebirth_mem)) == 0)
        return;

    IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER*)module_handle;
    IMAGE_NT_HEADERS *pe_header = (IMAGE_NT_HEADERS*)((DWORD)dos_header->e_lfanew + (DWORD)rebirth_mem.AllocationBase);
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE || pe_header->Signature != IMAGE_NT_SIGNATURE)
        return;

    module_address_ = (DWORD)rebirth_mem.AllocationBase;
    module_size_ = pe_header->OptionalHeader.SizeOfImage;

    // Find the static address pointer of the Rebirth PlayerManager instance
    std::vector<unsigned char> player_manager_inst_address_p_bytes_ = SearchMemForVal(PlayerManagerInstAddr);
    if (player_manager_inst_address_p_bytes_.size() < 4)
        return;
    player_manager_inst_p_addr_ = *((DWORD*)player_manager_inst_address_p_bytes_.data()) + 4;

    // Find the offset of the Players list relative to the PlayerManager instance
    *((DWORD*)(PlayerManagerPlayerListOffset.signature + 2)) = player_manager_inst_p_addr_;
    std::vector<unsigned char> player_manager_player_list_offset_bytes_ = SearchMemForVal(PlayerManagerPlayerListOffset);
    if (player_manager_player_list_offset_bytes_.size() < 2)
        return;
    player_manager_player_list_offset_ = *((WORD*)player_manager_player_list_offset_bytes_.data());
}

std::vector<unsigned char> RebirthMemReader::SearchMemForVal(MemSig mem_sig)
{
    std::vector<unsigned char> val_bytes;
    int sig_len = strlen(mem_sig.search_mask);
    unsigned char* p_search = (unsigned char*)module_address_;
    unsigned char* p_search_end = (unsigned char*)(module_address_ + module_size_ - sig_len);

    while (p_search < p_search_end)
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
