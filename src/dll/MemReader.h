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

#ifndef MISSINGHUD2_MEMREADER_H
#define MISSINGHUD2_MEMREADER_H

#include <vector>
#include <map>
#include <chrono>

#include <windows.h>

#include "IsaacMemSignatures.h"
#include "src/MHUD_MsgQueue.h"

#define WCHAR_ISAAC_MODULE_NAME   L"isaac-ng.exe"

enum Expansion
{
    kRebirth,
    kAfterbirth
};

// These values are the offsets of the specific statistic from the core Player memory address
enum PlayerStat
{
    kSpeed,
    kRange,
    kTearsDelay,
    kShotSpeed,
    kShotHeight,
    kDamage,
    kLuck,
    kTearsFired,
    kDealDoorChance, // An advanced function is required for this statistic
    kDealWithDevil,  // An advanced function is required for this statistic
    kDealWithAngel   // An advanced function is required for this statistic
};

struct RecentStatChange
{
    PlayerStat stat;
    float prev_stat_val = 0.0f;
    float new_stat_val = 0.0f;
    float stat_diff = 0.0f;
    std::chrono::time_point<std::chrono::system_clock> time_changed = std::chrono::system_clock::now();
    std::chrono::milliseconds show_timeout = std::chrono::milliseconds(3000);
};

struct ModuleInfo
{
    DWORD module_address = 0;
    DWORD module_size = 0;
};

class MemReader
{
public:
    static MemReader *GetMemoryReader();
    static void Destroy();

    // Functions that the expansion-specific memory readers must implement
    virtual bool IsRunActive() =0;
    virtual float GetPlayerStatf(PlayerStat player_stat) =0;
    virtual int GetPlayerStati(PlayerStat player_stat) =0;
    virtual float GetPlayerRecentStatChangef(PlayerStat player_stat) =0;
    virtual int GetPlayerRecentStatChangei(PlayerStat player_stat) =0;

    // Functions that the expansion-specific memory readers can implement
    virtual bool PlayingGreed() { return false; };

protected:
    static ModuleInfo GetModuleInfo();
    static Expansion GetIsaacExpansion();

    // Note this function always returns the first signature it finds, so make sure it's unique
    static std::vector<unsigned char> SearchMemForVal(MemSig mem_sig);
    static std::vector<unsigned char> SearchMemForVal(ModuleInfo module_info, MemSig mem_sig);

private:
    static MemReader *instance_;
    static ModuleInfo module_info_;
};

#endif //MISSINGHUD2_MEMREADER_H
