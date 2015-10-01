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

#ifndef MISSINGHUD2_REBIRTHMEMREADER_H
#define MISSINGHUD2_REBIRTHMEMREADER_H

#include <map>
#include <vector>
#include <sstream>
#include <chrono>

#include <easylogging++.h>
#include <windows.h>

#include "RebirthMemSignatures.h"

#define ISAAC_MODULE_NAME "isaac-ng.exe"

#define ITEM_ACTIVE_SLOT 0xCC8
#define ACTIVE_ITEM_BOOKOFREVELATIONS 0x4E
#define ACTIVE_ITEM_BOOKOFBELIAL 0x22

#define PASSIVE_ITEM_PENTAGRAM 0xE38
#define PASSIVE_ITEM_GOATHEAD 0x10C8
#define PASSIVE_ITEM_BLACKCANDLE 0x117C

#define PLAYER_MANAGER_FLOOR_FLAGS 0x5DC0
#define PLAYER_MANAGER_FLOOR_BOSS_FIGHT 0x5D98
#define PLAYER_MANAGER_ROOM_CODE 0x5D9C
#define PLAYER_MANAGER_BOSS_ROOM_CODE 0x5DA4
#define PLAYER_MANAGER_DEVILDEAL_PREV_FLOOR 0x10D7E8

#define BOSS_FIGHT_TOOK_RED_DMG 0xE8C

#define PLAYER_MANAGER_CURSE_FLAGS 0x8
#define LABYRINTH_CURSE 0x2

// These values are the offsets of the specific statistic from the core Player memory address
enum RebirthPlayerStat
{
    kSpeed = 0xCB4,
    kRange = 0xBF4,
    kTearsDelay = 0xBE0,
    kShotSpeed = 0xBE4,
    kDamage = 0xBF0,
    kLuck = 0xCB8,
    kDealWithDevil = 0xFFFFFFFF  // An advanced function is required for this statistic
};

struct RecentStatChange
{
    RebirthPlayerStat stat;
    float prev_stat_val = 0.0f;
    float new_stat_val = 0.0f;
    float stat_diff = 0.0f;
    std::chrono::time_point<std::chrono::system_clock> time_changed = std::chrono::system_clock::now();
    std::chrono::milliseconds show_timeout = std::chrono::milliseconds(3000);
};

class RebirthMemReader
{
public:
    static RebirthMemReader *GetMemoryReader();
    static void Destroy();

    bool IsRunActive();
    float GetPlayerStatf(RebirthPlayerStat player_stat);
    int GetPlayerStati(RebirthPlayerStat player_stat);

    float GetPlayerRecentStatChangef(RebirthPlayerStat player_stat);
    int GetPlayerRecentStatChangei(RebirthPlayerStat player_stat);

private:
    RebirthMemReader();
    ~RebirthMemReader();

    void GetRebirthModuleInfo();

    // Note this function always returns the first signature it finds, so make sure it's unique
    std::vector<unsigned char> SearchMemForVal(MemSig mem_sig);

    DWORD GetPlayerManagerMemAddr();
    DWORD GetPlayerListMemAddr();
    DWORD GetPlayerMemAddr();

    float GetDealWithDevilChance();
    DWORD GetCurrentRoom();

    void SaveStat(RebirthPlayerStat player_stat, float stat_val);

private:
    static RebirthMemReader* mem_reader_;

    DWORD module_address_ = 0;
    DWORD module_size_ = 0;

    DWORD player_manager_inst_p_addr_ = 0;
    DWORD player_manager_player_list_offset_ = 0;

    int current_floor_ = 0;
    bool boss_fight_took_dmg_ = false;
    std::map<RebirthPlayerStat, RecentStatChange> stat_change_;
};

#endif //MISSINGHUD2_REBIRTHMEMREADER_H
