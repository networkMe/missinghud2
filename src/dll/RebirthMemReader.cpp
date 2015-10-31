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
    // To check whether a run is currently active I check how many players there are (just like Rebirth actually does)
    // When there are 0 players, then we are not in a run
    DWORD player_list = GetPlayerListMemAddr();
    if (player_list == 0)
        return false;

    int num_players = (int)((*(DWORD*)(player_list + sizeof(DWORD))) - (*(DWORD*) player_list));
    return (num_players > 0);
}

float RebirthMemReader::GetPlayerStatf(RebirthPlayerStat player_stat)
{
    if (player_stat == RebirthPlayerStat::kDealWithDevil)
    {
        float devil_chance = GetDealWithDevilChance();
        SaveStat(RebirthPlayerStat::kDealWithDevil, devil_chance);
        return devil_chance;
    }

    DWORD player_class = GetPlayerMemAddr();
    if (player_class == 0)
        return 0.0f;

    float stat_val = *((float*)(player_class + player_stat));

    if (player_stat == RebirthPlayerStat::kRange)
    {
        stat_val *= -1; // Range is stored as a negative, but we show it positive on the HUD
    }

    // Save the stat into our memory
    SaveStat(player_stat, stat_val);
    return stat_val;
}

int RebirthMemReader::GetPlayerStati(RebirthPlayerStat player_stat)
{
    DWORD player_class = GetPlayerMemAddr();
    if (player_class == 0)
        return 0;

    int stat_val = *((int*)(player_class + player_stat));

    // Save the stat into our memory
    SaveStat(player_stat, (float)stat_val);
    return stat_val;
}

float RebirthMemReader::GetDealWithDevilChance()
{
    DWORD player_manager_inst = GetPlayerManagerMemAddr();
    if (player_manager_inst == 0)
        return 0.0f;

    int current_floor = *((int*)player_manager_inst);
    int labyrinth_flag = *((int*)((DWORD)player_manager_inst + PLAYER_MANAGER_CURSE_FLAGS)); // Need to take into account whether the floor is a labyrinth cursed floor
    current_floor_ = current_floor;
    if (labyrinth_flag == LABYRINTH_CURSE)
        ++current_floor_;
    if (current_floor_ == 1 || current_floor_ > 8)    // In-eligible for natural DWD on these floors (even with Goat Head)
        return 0.0f;

    // If these 2 conditions are met, you get a guaranteed DWD. I have not a clue what sets these though!
    DWORD dwd_floor_flag_unknown_1 = *((DWORD*)(player_manager_inst + PLAYER_MANAGER_DEVILDEAL_UNKNOWN_1));
    DWORD dwd_floor_flag_unknown_2 = *((DWORD*)(player_manager_inst + PLAYER_MANAGER_DEVILDEAL_UNKNOWN_2));
    if (dwd_floor_flag_unknown_1 == 0x2 && dwd_floor_flag_unknown_2 < 0xB)
        return 1.0f;

    DWORD player = GetPlayerMemAddr();
    float dwd_chance = 0.01f; // Default 1% chance

    if (PlayerHasItem(PASSIVE_ITEM_PENTAGRAM))      // Pentagram adds 10% chance
        dwd_chance += 0.10f;

    if (PlayerHasItem(PASSIVE_ITEM_BLACKCANDLE))    // Black Candle adds 15% chance
        dwd_chance += 0.15f;

    DWORD afterbirth_items_flag = *((DWORD*)(player + UNKNOWN_AFTERBIRTH_ITEMS));

    // No idea what Afterbirth item this is, but it can affect whether
    // the Afterbirth item's 5% increase is introduced
    if (PlayerHasItem(0x188))
    {
        // This item relies on RNG as to whether it contributes to the devil deal chance or not??
        DWORD rng_func_result = AfterBirthItemRNGFunc();
        if (rng_func_result == 0x33)
            ++afterbirth_items_flag;
    }

    if (afterbirth_items_flag > 1)   // There's a 3 Afterbirth items that use this flag
        dwd_chance += 0.05f;         // They add 5% if it's above 1 but I have no idea what sets this flag

    DWORD player_active_item = *((DWORD*)(player + ITEM_ACTIVE_SLOT));
    if (player_active_item == ACTIVE_ITEM_BOOKOFREVELATIONS)    // Holding Book of Revelations adds 17.5% chance
        dwd_chance += 0.175f;
    else if (player_active_item == ACTIVE_ITEM_BOOKOFBELIAL)    // Holding Book of Belial adds 12.5% chance
        dwd_chance += 0.125f;

    BYTE floor_flag = *((BYTE*)(player_manager_inst + PLAYER_MANAGER_FLOOR_FLAGS));
    if (floor_flag & 1)                                         // Killing a normal beggar adds 35% chance
        dwd_chance += 0.35f;

    DWORD floor_flags = *((DWORD*)(player_manager_inst + PLAYER_MANAGER_FLOOR_FLAGS));
    if (((floor_flags >> 2) & 1) <= 0)                          // Not taking red heart damage on the entire floor adds 99% chance
        dwd_chance += 0.99f;
    if (((floor_flags >> 6) & 1) > 0)                           // Blowing up a dead shopkeeper adds 10% chance
        dwd_chance += 0.10f;

    // Not taking damage from the floor's boss room adds 35% chance to DWD chance
    DWORD current_room = GetCurrentRoom();
    DWORD boss_room = *((DWORD*)(player_manager_inst + PLAYER_MANAGER_BOSS_ROOM_CODE));
    if (current_room == boss_room)
    {
        DWORD boss_fight = *((DWORD*)(player_manager_inst + PLAYER_MANAGER_FLOOR_BOSS_FIGHT));
        BYTE boss_dmg_flag = *((BYTE*)(boss_fight + BOSS_FIGHT_TOOK_RED_DMG));
        if (boss_dmg_flag == 1)
            boss_fight_took_dmg_ = true;
    }
    else
    {
        // This works to replicate how Rebirth handles boss fight damage.
        // On the first roll of the DWD chance it takes into account whether you took damage or not.
        // However, subsequent rolls (to keep the door open) ALWAYS add the +35% chance,
        // regardless of whether you took boss damage or not.
        if (boss_fight_took_dmg_)
            boss_fight_took_dmg_ = false;
    }

    if (!boss_fight_took_dmg_)      // Not taking damage from the boss fight adds 35% chance
        dwd_chance += 0.35f;

    DWORD devil_deal_prev_floor = *((DWORD*)(player_manager_inst + PLAYER_MANAGER_DEVILDEAL_PREV_FLOOR));
    if (devil_deal_prev_floor > 0)
    {
        int devil_deal_num_floors_ago = current_floor - (int)devil_deal_prev_floor; // Rebirth uses the non-labyrinthed current_floor value
        if (devil_deal_num_floors_ago <= 1)
        {
            dwd_chance *= 0.25f; // Player has seen a Deal with Devil door less than 2 floors ago, reduce overall chance by 75%
        }
        else if (devil_deal_num_floors_ago == 2)
        {
            dwd_chance *= 0.5f; // Player has seen a Deal with Devil door 2 floors ago, reduce overall chance by 50%
        }
    }

    if (PlayerHasItem(PASSIVE_ITEM_GOATHEAD))       // Goat Head adds 666% chance (nice!)
        dwd_chance += 66.6;

    if (dwd_chance > 1.00f)
        dwd_chance = 1.00f;

    return dwd_chance;
}

void RebirthMemReader::SaveStat(RebirthPlayerStat player_stat, float stat_val)
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

float RebirthMemReader::GetPlayerRecentStatChangef(RebirthPlayerStat player_stat)
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

int RebirthMemReader::GetPlayerRecentStatChangei(RebirthPlayerStat player_stat)
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

DWORD RebirthMemReader::GetCurrentRoom()
{
    DWORD player_manager_inst = GetPlayerManagerMemAddr();
    if (player_manager_inst == 0)
        return 0;

    DWORD room_code = *((DWORD*)(player_manager_inst + PLAYER_MANAGER_ROOM_CODE));
    DWORD room_num = *((DWORD*)(player_manager_inst + (room_code * 4) + PLAYER_MANAGER_ROOM_CODE_FORMULA_OFFSET));

    return room_num;
}

void RebirthMemReader::GetRebirthModuleInfo()
{
    // Get the base address of the Rebirth module
    DWORD module_handle = (DWORD)GetModuleHandleW(WCHAR_ISAAC_MODULE_NAME);
    MEMORY_BASIC_INFORMATION rebirth_mem = { 0 };
    if (VirtualQuery((LPVOID)module_handle, &rebirth_mem, sizeof(rebirth_mem)) == 0)
        throw std::runtime_error("Unable to get memory information for Isaac.");

    IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER*)module_handle;
    IMAGE_NT_HEADERS *pe_header = (IMAGE_NT_HEADERS*)((DWORD)dos_header->e_lfanew + (DWORD)rebirth_mem.AllocationBase);
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE || pe_header->Signature != IMAGE_NT_SIGNATURE)
        throw std::runtime_error("The Rebirth memory being accessed is incorrect.");

    module_address_ = (DWORD)rebirth_mem.AllocationBase;
    module_size_ = pe_header->OptionalHeader.SizeOfImage;

    std::stringstream ss;
    ss << "Rebirth module address: 0x" << std::hex << module_address_;
    QUEUE_LOG(mhud2::Log::LOG_INFO, ss.str());
    ss.str(""); ss.clear();

    ss << "Rebirth module size: " << std::hex << module_size_;
    QUEUE_LOG(mhud2::Log::LOG_INFO, ss.str());
    ss.str(""); ss.clear();

    // Find the static address pointer of the Rebirth PlayerManager instance
    std::vector<unsigned char> player_manager_inst_address_p_bytes_ = SearchMemForVal(PlayerManagerInstAddr);
    if (player_manager_inst_address_p_bytes_.size() < 4)
        throw std::runtime_error("Couldn't find the PlayerManager static instance address");
    player_manager_inst_p_addr_ = *((DWORD*)player_manager_inst_address_p_bytes_.data());

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

    // Find the map address for the RNG fucntion
    std::vector<unsigned char> rng_map_address = SearchMemForVal(AfterbirthRNGMap);
    if (rng_map_address.size() < 4)
        throw std::runtime_error("Couldn't find the RNG map address!");
    rng_map_addr_ = *((DWORD*)rng_map_address.data());

    ss << "RNG Map offset: " << std::hex << rng_map_addr_;
    QUEUE_LOG(mhud2::Log::LOG_INFO, ss.str());
    ss.str(""); ss.clear();

    // Find the RNG addresses for the RNG function
    std::vector<unsigned char> rng_value_addresses = SearchMemForVal(AfterbirthRNGVals);
    if (rng_value_addresses.size() < 12)
        throw std::runtime_error("Couldn't find the RNG value addresses!");
    rng_value_1_addr_ = *((DWORD*)rng_value_addresses.data());
    rng_value_2_addr_ = *((DWORD*)(rng_value_addresses.data() + 0x4));
    rng_value_3_addr_ = *((DWORD*)(rng_value_addresses.data() + 0x8));

    ss << "RNG Value 1 offset: " << std::hex << rng_value_1_addr_;
    QUEUE_LOG(mhud2::Log::LOG_INFO, ss.str());
    ss.str(""); ss.clear();

    ss << "RNG Value 2 offset: " << std::hex << rng_value_2_addr_;
    QUEUE_LOG(mhud2::Log::LOG_INFO, ss.str());
    ss.str(""); ss.clear();

    ss << "RNG Value 3 offset: " << std::hex << rng_value_3_addr_;
    QUEUE_LOG(mhud2::Log::LOG_INFO, ss.str());
    ss.str(""); ss.clear();
}

std::vector<unsigned char> RebirthMemReader::SearchMemForVal(MemSig mem_sig)
{
    std::vector<unsigned char> val_bytes;
    int sig_len = strlen(mem_sig.search_mask);
    unsigned char* p_search = (unsigned char*)module_address_;
    unsigned char* p_search_end = (unsigned char*)(module_address_ + module_size_ - sig_len);

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

bool RebirthMemReader::PlayerHasItem(int item_id)
{
    DWORD player = GetPlayerMemAddr();
    DWORD item_flag = *((DWORD*)(player + (item_id * 4) + PLAYER_HAS_ITEM_FORM_OFFSET));

    return (item_flag != 0);
}

bool RebirthMemReader::PlayingGreed()
{
    DWORD player_manager_inst = GetPlayerManagerMemAddr();
    if (player_manager_inst == 0)
        return false;

    int game_mode_flag = *((int*)((DWORD)player_manager_inst + PLAYER_MANAGER_GAME_MODE_FLAG));
    return (game_mode_flag == GREED_GAME_MODE);
}

DWORD RebirthMemReader::AfterBirthItemRNGFunc()
{
    if (!PlayerHasItem(0x188))
        return 0;

    DWORD player_manager_inst = GetPlayerManagerMemAddr();
    if (player_manager_inst == 0)
        return 0;

    DWORD game_seed = *((DWORD*)(player_manager_inst + 0xB848));
    if (game_seed == 0)
        return 0;

    DWORD current_floor = *((DWORD*)(player_manager_inst));
    DWORD rng_addr = ((DWORD)(player_manager_inst + 0xB844));
    DWORD rng_seed = *((DWORD*)(rng_addr + (current_floor*4) + 0x18));
    DWORD rng_seed_1 = *(DWORD*)rng_value_1_addr_;
    DWORD rng_seed_2 = *(DWORD*)rng_value_2_addr_;
    DWORD rng_seed_3 = *(DWORD*)rng_value_3_addr_;

    // RNG algorithm
    DWORD rng_1 = (rng_seed >> rng_seed_2) ^ rng_seed;
    DWORD rng_2 = rng_1 ^ (rng_1 << rng_seed_3);
    DWORD rng_3 = rng_2 ^ (rng_2 >> rng_seed_1);
    uint64_t rng_multiplied = rng_3 * (uint64_t)0xAAAAAAAB;
    DWORD rng_4 = (rng_multiplied >> 32) >> 3;
    DWORD rng_map_base = rng_4 + (rng_4 * 2);
    rng_map_base += rng_map_base;
    rng_map_base += rng_map_base;
    DWORD map_modifier = rng_3 - rng_map_base;

    DWORD rng_result = *((DWORD*)((map_modifier * 4) + rng_map_addr_));
    return rng_result;
}
