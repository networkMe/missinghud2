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

#include "MemReader.h"

#define RB_ITEM_ACTIVE_SLOT 0xCC8
#define RB_ACTIVE_ITEM_BOOKOFREVELATIONS 0x4E
#define RB_ACTIVE_ITEM_BOOKOFBELIAL 0x22

#define RB_PASSIVE_ITEM_PENTAGRAM 0xE38
#define RB_PASSIVE_ITEM_GOATHEAD 0x10C8
#define RB_PASSIVE_ITEM_BLACKCANDLE 0x117C
#define RB_PASSIVE_ITEM_KEYPIECE_1 0x1124
#define RB_PASSIVE_ITEM_KEYPIECE_2 0x1128
#define RB_PASSIVE_ITEM_MUMS_PURSE 0xF98

#define RB_PLAYER_HAS_TRINKET_OFFSET 0xD64
#define RB_PASSIVE_TRINKET_ROSARYBEAD 0x7

#define RB_PLAYER_MANAGER_CURSE_FLAGS 0x8
#define RB_PLAYER_MANAGER_FLOOR_FLAGS 0x5DC0
#define RB_PLAYER_MANAGER_BOSS_ROOM_CODE 0x5DA4
#define RB_PLAYER_MANAGER_ROOM_CODE 0x5D9C
#define RB_PLAYER_MANAGER_ROOM_CODE_FORMULA_OFFSET 0x5AC4
#define RB_PLAYER_MANAGER_FLOOR_BOSS_FIGHT 0x5D98
#define RB_PLAYER_MANAGER_DEAL_PREV_FLOOR 0x10D7E8
#define RB_PLAYER_MANAGER_FLOOR_DONATIONS 0x10D800
#define RB_PLAYER_MANAGER_SEEN_DEVIL 0x10D7E0
#define RB_PLAYER_MANAGER_PAID_DEVIL 0x10D7F8

#define RB_LABYRINTH_CURSE 0x2
#define RB_BOSS_FIGHT_TOOK_RED_DMG 0xE8C

#define RB_STAT_SPEED 0xCB4
#define RB_STAT_RANGE 0xBF4
#define RB_STAT_TEARS 0xBE0
#define RB_STAT_SHOTSPEED 0xBE4
#define RB_STAT_SHOTHEIGHT 0xBF8
#define RB_STAT_DAMAGE 0xBF0
#define RB_STAT_LUCK 0xCB8
#define RB_STAT_TEARSFIRED 0xBEC

class RebirthMemReader : public MemReader
{
public:
    RebirthMemReader();
    ~RebirthMemReader();

    bool IsRunActive();

    float GetPlayerStatf(PlayerStat player_stat);
    int GetPlayerStati(PlayerStat player_stat);

    float GetPlayerRecentStatChangef(PlayerStat player_stat);
    int GetPlayerRecentStatChangei(PlayerStat player_stat);

private:
    void GetRebirthModuleInfo();

    DWORD GetPlayerManagerMemAddr();
    DWORD GetPlayerListMemAddr();
    DWORD GetPlayerMemAddr();

    float GetDealDoorChance();
    float GetDealWithDevilChance();
    float GetDealWithAngelChance();
    float GetDealWithAngelMultiplier();
    DWORD GetCurrentRoom();

    bool PlayerHasItem(DWORD item_offset);
    bool PlayerHasTrinket(int trinket_id);

    void SaveStat(PlayerStat player_stat, float stat_val);

private:
    DWORD player_manager_inst_p_addr_ = 0;
    DWORD player_manager_player_list_offset_ = 0;

    int current_floor_ = 0;
    bool boss_fight_took_dmg_ = false;
    std::map<PlayerStat, RecentStatChange> stat_change_;
};


#endif //MISSINGHUD2_REBIRTHMEMREADER_H
