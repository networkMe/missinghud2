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

RebirthMemReader::RebirthMemReader()
{
    GetRebirthModuleInfo();
}

RebirthMemReader::~RebirthMemReader()
{
}

void RebirthMemReader::GetRebirthModuleInfo()
{
    // Find the static address pointer of the Rebirth PlayerManager instance
    std::vector<unsigned char> player_manager_inst_address_p_bytes_ = SearchMemForVal(PlayerManagerInstAddr);
    if (player_manager_inst_address_p_bytes_.size() < 4)
        throw std::runtime_error("Couldn't find the PlayerManager static instance address");
    player_manager_inst_p_addr_ = *((DWORD*)player_manager_inst_address_p_bytes_.data());

    std::stringstream ss;
    ss << "PlayerManager Instance **: " << std::hex << player_manager_inst_p_addr_;
    QUEUE_LOG(mhud2::Log::LOG_INFO, ss.str());
    ss.str(""); ss.clear();

    // Find the offset of the Players list relative to the PlayerManager instance
    *((DWORD*)(PlayerManagerPlayerListOffset.signature + 2)) = player_manager_inst_p_addr_;
    std::vector<unsigned char> player_manager_player_list_offset_bytes_ = SearchMemForVal(PlayerManagerPlayerListOffset);
    if (player_manager_player_list_offset_bytes_.size() < 2)
        throw std::runtime_error("Couldn't find the PlayerManager PlayerList offset");
    player_manager_player_list_offset_ = *((WORD*)player_manager_player_list_offset_bytes_.data());

    ss << "PlayerManager PlayerList offset: " << std::hex << player_manager_player_list_offset_;
    QUEUE_LOG(mhud2::Log::LOG_INFO, ss.str());
    ss.str(""); ss.clear();
}

bool RebirthMemReader::IsRunActive()
{
    // To check whether a run is currently active I check how many players there are (just like Afterbirth actually does)
    // When there are 0 players, then we are not in a run
    DWORD player_list = GetPlayerListMemAddr();
    if (player_list == 0)
        return false;

    int num_players = (int)((*(DWORD*)(player_list + sizeof(DWORD))) - (*(DWORD*) player_list));
    return (num_players > 0);
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

float RebirthMemReader::GetPlayerStatf(PlayerStat player_stat)
{
    if (player_stat == PlayerStat::kDealDoorChance)
    {
        float door_chance = GetDealDoorChance();
        SaveStat(PlayerStat::kDealDoorChance, door_chance);
        return door_chance;
    }
    else if (player_stat == PlayerStat::kDealWithDevil)
    {
        float devil_chance = GetDealWithDevilChance();
        SaveStat(PlayerStat::kDealWithDevil, devil_chance);
        return devil_chance;
    }
    else if (player_stat == PlayerStat::kDealWithAngel)
    {
        float angel_chance = GetDealWithAngelChance();
        SaveStat(PlayerStat::kDealWithAngel, angel_chance);
        return angel_chance;
    }

    DWORD player_class = GetPlayerMemAddr();
    if (player_class == 0)
        return 0.0f;

    float stat_val;
    switch (player_stat)
    {
        case PlayerStat::kSpeed:
            stat_val = *((float*)(player_class + RB_STAT_SPEED));
            break;

        case PlayerStat::kRange:
            stat_val = *((float*)(player_class + RB_STAT_RANGE));
            break;

        case PlayerStat::kShotSpeed:
            stat_val = *((float*)(player_class + RB_STAT_SHOTSPEED));
            break;

        case PlayerStat::kShotHeight:
            stat_val = *((float*)(player_class + RB_STAT_SHOTHEIGHT));
            break;

        case PlayerStat::kDamage:
            stat_val = *((float*)(player_class + RB_STAT_DAMAGE));
            break;

        case PlayerStat::kLuck:
            stat_val = *((float*)(player_class + RB_STAT_LUCK));
            break;

        default:
            stat_val = 0.00f;
    }

    if (player_stat == PlayerStat::kRange)
    {
        stat_val *= -1; // Range is stored as a negative, but we show it positive on the HUD
    }

    // Save the stat into our memory
    SaveStat(player_stat, stat_val);
    return stat_val;
}

int RebirthMemReader::GetPlayerStati(PlayerStat player_stat)
{
    DWORD player_class = GetPlayerMemAddr();
    if (player_class == 0)
        return 0;

    int stat_val;
    switch (player_stat)
    {
        case PlayerStat::kTearsDelay:
            stat_val = *((int*)(player_class + RB_STAT_TEARS));
            break;

        case PlayerStat::kTearsFired:
            stat_val = *((int*)(player_class + RB_STAT_TEARSFIRED));
            break;

        default:
            stat_val = 0;
    }

    // Save the stat into our memory
    SaveStat(player_stat, (float)stat_val);
    return stat_val;
}

float RebirthMemReader::GetPlayerRecentStatChangef(PlayerStat player_stat)
{
    float recent_stat_change = 0.0f;

    if (stat_change_.count(player_stat) > 0)
    {
        if (stat_change_[player_stat].time_changed > (std::chrono::system_clock::now() - stat_change_[player_stat].show_timeout))
        {
            return stat_change_[player_stat].stat_diff;
        }
    }

    return recent_stat_change;
}

int RebirthMemReader::GetPlayerRecentStatChangei(PlayerStat player_stat)
{
    int recent_stat_change = 0;

    if (stat_change_.count(player_stat) > 0)
    {
        if (stat_change_[player_stat].time_changed > (std::chrono::system_clock::now() - stat_change_[player_stat].show_timeout))
        {
            return (int)stat_change_[player_stat].stat_diff;
        }
    }

    return recent_stat_change;
}

void RebirthMemReader::SaveStat(PlayerStat player_stat, float stat_val)
{
    // If the stat is different from the one in our memory, note it
    if (stat_change_.count(player_stat) > 0)
    {
        if (stat_change_[player_stat].new_stat_val != stat_val)
        {
            stat_change_[player_stat].prev_stat_val = stat_change_[player_stat].new_stat_val;
            stat_change_[player_stat].new_stat_val = stat_val;
            stat_change_[player_stat].stat_diff = stat_change_[player_stat].new_stat_val - stat_change_[player_stat].prev_stat_val;
            stat_change_[player_stat].time_changed = std::chrono::system_clock::now();
        }
    }
    else
    {
        RecentStatChange new_stat_change;
        new_stat_change.stat = player_stat;
        new_stat_change.prev_stat_val = stat_val;
        new_stat_change.new_stat_val = stat_val;
        new_stat_change.time_changed = new_stat_change.time_changed - new_stat_change.show_timeout; // We don't want the initial 0->X change to show
        stat_change_[player_stat] = new_stat_change;
    }
}

float RebirthMemReader::GetDealWithDevilChance()
{
    return GetDealDoorChance() * (1.00f - GetDealWithAngelMultiplier());
}

float RebirthMemReader::GetDealWithAngelChance()
{
    return GetDealDoorChance() * GetDealWithAngelMultiplier();
}

float RebirthMemReader::GetDealDoorChance()
{
    DWORD player_manager_inst = GetPlayerManagerMemAddr();
    if (player_manager_inst == 0)
        return 0.0f;

    int current_floor = *((int*)player_manager_inst);
    int labyrinth_flag = *((int*)((DWORD)player_manager_inst + RB_PLAYER_MANAGER_CURSE_FLAGS));    // Need to take into account whether the floor is a labyrinth cursed floor
    current_floor_ = current_floor;
    if ((current_floor_ == 1 && labyrinth_flag != RB_LABYRINTH_CURSE) || current_floor_ > 8)    // In-eligible for natural deal on these floors (even with Goat Head)
        return 0.0f;

    DWORD player = GetPlayerMemAddr();
    if (PlayerHasItem(RB_PASSIVE_ITEM_GOATHEAD))       // Goat Head is a guaranteed deal (100%)
        return 1.0f;

    float deal_chance = 0.01f; // Default 1% chance

    if (PlayerHasItem(RB_PASSIVE_ITEM_PENTAGRAM))                  // Pentagram adds 20% chance
        deal_chance += 0.20f;

    if (*((DWORD*)(player + RB_PASSIVE_ITEM_PENTAGRAM)) > 1)       // Having more than one pentagram adds another 10% chance
        deal_chance += 0.10f;

    if (PlayerHasItem(RB_PASSIVE_ITEM_BLACKCANDLE))    // Black Candle adds 30% chance
        deal_chance += 0.30f;

    DWORD player_active_item = *((DWORD*)(player + RB_ITEM_ACTIVE_SLOT));
    if (player_active_item == RB_ACTIVE_ITEM_BOOKOFREVELATIONS)    // Holding Book of Revelations adds 35% chance
        deal_chance += 0.35f;
    else if (player_active_item == RB_ACTIVE_ITEM_BOOKOFBELIAL)    // Holding Book of Belial adds 2500% chance
        deal_chance += 25.00f;

    BYTE floor_flag = *((BYTE*)(player_manager_inst + RB_PLAYER_MANAGER_FLOOR_FLAGS));
    if (floor_flag & 1)                                         // Killing a normal beggar adds 35% chance
        deal_chance += 0.35f;

    DWORD floor_flags = *((DWORD*)(player_manager_inst + RB_PLAYER_MANAGER_FLOOR_FLAGS));
    if (((floor_flags >> 2) & 1) <= 0)                          // Not taking red heart damage on the entire floor adds 99% chance
        deal_chance += 0.99f;
    if (((floor_flags >> 6) & 1) > 0)                           // Blowing up a dead shopkeeper adds 10% chance
        deal_chance += 0.10f;

    // Not taking damage from the floor's boss room adds 35% chance to deal chance
    DWORD current_room = GetCurrentRoom();
    DWORD boss_room = *((DWORD*)(player_manager_inst + RB_PLAYER_MANAGER_BOSS_ROOM_CODE));
    if (current_room == boss_room)
    {
        DWORD boss_fight = *((DWORD*)(player_manager_inst + RB_PLAYER_MANAGER_FLOOR_BOSS_FIGHT));
        BYTE boss_dmg_flag = *((BYTE*)(boss_fight + RB_BOSS_FIGHT_TOOK_RED_DMG));
        if (boss_dmg_flag == 1)
            boss_fight_took_dmg_ = true;
    }
    else
    {
        // This works to replicate how Rebirth handles boss fight damage.
        // On the first roll of the deal chance it takes into account whether you took damage or not.
        // However, subsequent rolls (to keep the door open) ALWAYS add the +35% chance,
        // regardless of whether you took boss damage or not.
        if (boss_fight_took_dmg_)
            boss_fight_took_dmg_ = false;
    }

    if (!boss_fight_took_dmg_)      // Not taking damage from the boss fight adds 35% chance
        deal_chance += 0.35f;

    DWORD deal_prev_floor = *((DWORD*)(player_manager_inst + RB_PLAYER_MANAGER_DEAL_PREV_FLOOR));
    if (deal_prev_floor > 0)
    {
        int deal_num_floors_ago = current_floor - (int) deal_prev_floor; // Rebirth uses the non-labyrinthed current_floor value
        if (deal_num_floors_ago < 2)
        {
            deal_chance *= 0.25f; // Player has seen a Deal door less than 2 floors ago, reduce overall chance by 75%
        }
        else if (deal_num_floors_ago == 2)
        {
            deal_chance *= 0.5f; // Player has seen a Deal door 2 floors ago, reduce overall chance by 50%
        }
    }

    if (deal_chance > 1.00f)
        deal_chance = 1.00f;

    return deal_chance;
}

DWORD RebirthMemReader::GetCurrentRoom()
{
    DWORD player_manager_inst = GetPlayerManagerMemAddr();
    if (player_manager_inst == 0)
        return 0;

    DWORD room_code = *((DWORD*)(player_manager_inst + RB_PLAYER_MANAGER_ROOM_CODE));
    DWORD room_num = *((DWORD*)(player_manager_inst + (room_code * 4) + RB_PLAYER_MANAGER_ROOM_CODE_FORMULA_OFFSET));
    return room_num;
}

float RebirthMemReader::GetDealWithAngelMultiplier()
{
    // If you haven't seen a devil deal yet, or you have taken a devil deal via health payment
    // you can't receive an Angel room
    DWORD player_manager_inst = GetPlayerManagerMemAddr();
    if (player_manager_inst == 0)
        return 0.0f;

    DWORD seen_devil = *((DWORD*)(player_manager_inst + RB_PLAYER_MANAGER_SEEN_DEVIL));
    if (((seen_devil & 0x20) | 0) == 0)
        return 0.0f;    // Haven't seen a Devil room, can't get Angel room yet

    if (*((DWORD*)(player_manager_inst + RB_PLAYER_MANAGER_PAID_DEVIL)) != 0)
        return 0.0f;    // Paid for a Devil deal with red health, can't get Angel room

    float angel_chance = 0.50f;    // Default chance to replace Devil room with Angel room is 50%

    DWORD player = GetPlayerMemAddr();
    if (PlayerHasItem(RB_PASSIVE_ITEM_KEYPIECE_1))
        angel_chance = angel_chance + ((1.00f - angel_chance) * 0.25f);    // Having Key Piece #1 gives a 25% chance roll

    if (PlayerHasItem(RB_PASSIVE_ITEM_KEYPIECE_2))
        angel_chance = angel_chance + ((1.00f - angel_chance) * 0.25f);    // Having Key Piece #2 gives a 25% chance roll

    if (PlayerHasTrinket(RB_PASSIVE_TRINKET_ROSARYBEAD))
        angel_chance = angel_chance + ((1.00f - angel_chance) * 0.50f);    // Holding the Rosary Bead trinket gives 50% chance roll

    if (*((DWORD*)(player_manager_inst + RB_PLAYER_MANAGER_FLOOR_DONATIONS)) >= 10)
        angel_chance = angel_chance + ((1.00f - angel_chance) * 0.50f);    // Donating 10 or more coins on a floor gives 50% chance roll

    DWORD floor_flags = *((DWORD*)(player_manager_inst + RB_PLAYER_MANAGER_FLOOR_FLAGS));
    if (((floor_flags >> 1) & 0xff) & 1 || ((floor_flags >> 3) & 0xff) & 1 || ((floor_flags >> 4) & 0xff) & 1)
        angel_chance = angel_chance + ((1.00f - angel_chance) * 0.25f);    // Devil Beggar / Shell Beggar / Key Beggar blown up
                                                                           // Gives 25% chance roll

    return angel_chance;
}

bool RebirthMemReader::PlayerHasItem(DWORD item_offset)
{
    DWORD player = GetPlayerMemAddr();
    return (*((DWORD*)(player + item_offset)) != 0);
}

bool RebirthMemReader::PlayerHasTrinket(int trinket_id)
{
    int num_trinkets = 1;
    if (PlayerHasItem(RB_PASSIVE_ITEM_MUMS_PURSE))
        num_trinkets++;    // Mum's Purse lets a player have 2 possible trinkets

    DWORD player = GetPlayerMemAddr();
    DWORD trinket_offset = 0;
    for (int i = 0; i < num_trinkets; ++i)
    {
        DWORD trinket_flag = *((DWORD*)(player + RB_PLAYER_HAS_TRINKET_OFFSET + trinket_offset));
        if (trinket_flag == trinket_id)
            return true;

        trinket_offset += 0x4;
    }

    // Player clearly doesn't have that trinket!
    return false;
}
