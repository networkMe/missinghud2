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

#include <vector>
#include <sstream>

#include <windows.h>

#include "RebirthMemSignatures.h"

#define ISAAC_MODULE_NAME "isaac-ng.exe"

// These values are the offsets of the specific statistic from the core Player memory address
enum RebirthPlayerStat {
    kSpeed = 0xCB4,
    kRange = 0xBF4,
    kTearsDelay = 0xBE0,
    kShotSpeed = 0xBE4,
    kDamage = 0xBF0,
    kLuck = 0xCB8,
    kDealWithDevil = 0x10C8  // This is for a Goathead guranteed DWD, actual DWD chance is much more complicated
};

class RebirthMemReader
{
public:
    static RebirthMemReader *GetMemoryReader();
    static void Destroy();

    bool IsRunActive();
    float GetPlayerStatf(RebirthPlayerStat player_stat);
    int GetPlayerStati(RebirthPlayerStat player_stat);

private:
    RebirthMemReader();
    ~RebirthMemReader();

    void GetRebirthModuleInfo();

    // Note this function always returns the first signature it finds, so make sure it's unique
    std::vector<unsigned char> SearchMemForVal(MemSig mem_sig);

    DWORD GetPlayerManagerMemAddr();
    DWORD GetPlayerListMemAddr();
    DWORD GetPlayerMemAddr();

private:
    static RebirthMemReader* mem_reader_;

    DWORD module_address_ = 0;
    DWORD module_size_ = 0;

    DWORD player_manager_inst_p_addr_ = 0;
    DWORD player_manager_player_list_offset_ = 0;
};


#endif //MISSINGHUD2_REBIRTHMEMREADER_H
