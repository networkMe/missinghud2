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

#ifndef MISSINGHUD2_AFTERBIRTHMEMREADER_H
#define MISSINGHUD2_AFTERBIRTHMEMREADER_H

#include <map>
#include <vector>
#include <sstream>
#include <chrono>

#include <windows.h>

#include "MemReader.h"
#include "IsaacMemSignatures.h"
#include "src/MHUD_MsgQueue.h"

#define AB_ITEM_ACTIVE_SLOT 0x1CF4
#define AB_ACTIVE_ITEM_BOOKOFREVELATIONS 0x4E
#define AB_ACTIVE_ITEM_BOOKOFBELIAL 0x22

#define AB_PLAYER_HAS_ITEM_FORM_OFFSET 0x1DA4
#define AB_PASSIVE_ITEM_PENTAGRAM 0x33
#define AB_PASSIVE_ITEM_BLACKCANDLE 0x104
#define AB_PASSIVE_ITEM_GOATHEAD 0xD7
#define AB_PASSIVE_ITEM_ZODIAC 0x188
#define AB_PASSIVE_ITEM_PENTAGRAM_COUNT 0x1E70
#define AB_PASSIVE_ITEM_KEYPIECE_1 0xEE
#define AB_PASSIVE_ITEM_KEYPIECE_2 0xEF
#define AB_PASSIVE_ITEM_MUMS_PURSE 0x8B

#define AB_PLAYER_HAS_TRINKET_OFFSET 0x1D9C
#define AB_PASSIVE_TRINKET_ROSARYBEAD 0x7

#define AB_PLAYER_MANAGER_FLOOR_FLAGS 0x708C
#define AB_PLAYER_MANAGER_FLOOR_BOSS_FIGHT 0x7014
#define AB_PLAYER_MANAGER_ROOM_CODE 0x7018
#define AB_PLAYER_MANAGER_ROOM_CODE_FORMULA_OFFSET 0x6D40
#define AB_PLAYER_MANAGER_BOSS_ROOM_CODE 0x7020
#define AB_PLAYER_MANAGER_DEAL_PREV_FLOOR 0x174DDC
#define AB_PLAYER_MANAGER_AMOUNT_DONATED 0x174DF4
#define AB_PLAYER_MANAGER_SEEN_DEVIL 0x174DD0
#define AB_PLAYER_MANAGER_PAID_DEVIL 0x174DEC

#define AB_BOSS_FIGHT_TOOK_RED_DMG 0xE8C

#define AB_PLAYER_MANAGER_CURSE_FLAGS 0xC
#define AB_PLAYER_MANAGER_GAME_MODE_FLAG 0x4
#define AB_GREED_GAME_MODE 0x3
#define AB_LABYRINTH_CURSE 0x2

#define AB_STAT_SPEED 0x1CDC
#define AB_STAT_RANGE 0x1C14
#define AB_STAT_TEARS 0x1C00
#define AB_STAT_SHOTSPEED 0x1C04
#define AB_STAT_SHOTHEIGHT 0x1C18
#define AB_STAT_DAMAGE 0x1C10
#define AB_STAT_LUCK 0x1CE0
#define AB_STAT_TEARSFIRED 0x1C0C

class AfterbirthMemReader : public MemReader
{
public:
    AfterbirthMemReader();
    ~AfterbirthMemReader();

    bool IsRunActive();
    bool PlayingGreed();

    float GetPlayerStatf(PlayerStat player_stat);
    int GetPlayerStati(PlayerStat player_stat);

    float GetPlayerRecentStatChangef(PlayerStat player_stat);
    int GetPlayerRecentStatChangei(PlayerStat player_stat);

private:
    void GetAfterbirthModuleInfo();

    DWORD GetPlayerManagerMemAddr();
    DWORD GetPlayerListMemAddr();
    DWORD GetPlayerMemAddr();

    float GetDealDoorChance();
    float GetDealWithDevilChance();
    float GetDealWithAngelChance();
    float GetDealWithAngelMultiplier();
    DWORD GetCurrentRoom();

    bool PlayerHasItem(int item_id);
    bool PlayerHasTrinket(int trinket_id);
    DWORD ZodiacItemRNGFunc();

    void SaveStat(PlayerStat player_stat, float stat_val);

private:
    DWORD player_manager_inst_p_addr_ = 0;
    DWORD player_manager_player_list_offset_ = 0;

    int current_floor_ = 0;
    bool boss_fight_took_dmg_ = false;
    std::map<PlayerStat, RecentStatChange> stat_change_;

    DWORD rng_map_addr_ = 0;
    DWORD rng_value_1_addr_ = 0;
    DWORD rng_value_2_addr_ = 0;
    DWORD rng_value_3_addr_ = 0;
};

#endif //MISSINGHUD2_AFTERBIRTHMEMREADER_H
