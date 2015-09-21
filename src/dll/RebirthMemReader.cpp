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
    // Get the base address of our Isaac module
    base_address_ = (DWORD)GetModuleHandle(ISAAC_MODULE_NAME);
}

RebirthMemReader::~RebirthMemReader()
{

}

bool RebirthMemReader::IsRunActive()
{
    // To check wether a run is currently active I check how many player managers there are
    // (just like Rebirth actually does)
    // When there are 0 player managers, then we are not in a run
    DWORD player_manager_class = GetPlayerManagerClassMemAddr();
    if (player_manager_class == 0)
        return false;

    int num_players = (int)((*(DWORD*)(player_manager_class + sizeof(DWORD))) - (*(DWORD*)player_manager_class));
    return (num_players > 0);
}

float RebirthMemReader::GetPlayerStatf(RebirthPlayerStat player_stat)
{
    DWORD player_class = GetPlayerClassMemAddr();
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
    DWORD player_class = GetPlayerClassMemAddr();
    if (player_class == 0)
        return 0;

    int stat_val = *((int*)(player_class + player_stat));
    return stat_val;
}

DWORD RebirthMemReader::GetPlayerManagerClassMemAddr()
{
    DWORD player_manager_p_class = (base_address_ + 0x21A1F4);
    DWORD player_manager_data_addr = *((DWORD*)(player_manager_p_class));
    if (player_manager_data_addr == 0)
        return 0;
    else
        return player_manager_data_addr + 0x8A2C;
}

DWORD RebirthMemReader::GetPlayerClassMemAddr()
{
    // Here we follow the Rebirth memory address chain to get
    // the current player class associated with the current run
    DWORD player_manager_class = GetPlayerManagerClassMemAddr();
    if (player_manager_class == 0)
        return 0;

    DWORD player_p_class = *((DWORD*)player_manager_class);
    return *((DWORD*)player_p_class);
}
