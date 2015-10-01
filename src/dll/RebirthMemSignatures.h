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

#ifndef MISSINGHUD2_REBIRTHMEMSIGNATURES_H
#define MISSINGHUD2_REBIRTHMEMSIGNATURES_H

struct MemSig
{
    const unsigned char* signature;      // As a byte array { 0x11, 0x22, 0x00, etc }
    const char* search_mask;             // As a string where "b" = actual byte to search for from the signature array,
                                         //                   "?" = a wildcard byte,
                                         //                   "v" = a value byte to return
};



// =============================================================
// The below signatures are valid for Isaac Rebirth 1.00 -> 1.05
// =============================================================



// 33 C0                 | xor eax,eax                          |
// C7 45 FC FF FF FF FF  | mov dword ptr ss:[ebp-4],FFFFFFFF    |
// A3 F4 A1 3E 01        | mov dword ptr ds:[13EA1F4],eax       |  <== 13EA1F4 is the address we want
// E8 30 EF F7 FF        | call isaac-ng.127C490                |
const static unsigned char PlayerManagerInstAddrSig[] =
{
    0x33, 0xC0,
    0xC7, 0x45, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF,
    0xA3, 0x00, 0x00, 0x00, 0x00,
    0xE8
};
const static MemSig PlayerManagerInstAddr = {
    PlayerManagerInstAddrSig,
    "bbbbbbbbbbvvvvb"
};



// 8B 35 F4 A1 33 01  | mov esi,dword ptr ds:[133A1F4]     |
// 8B 86 30 8A 00 00  | mov eax,dword ptr ds:[esi+8A30]    |
// 2B 86 2C 8A 00 00  | sub eax,dword ptr ds:[esi+8A2C]    |  <== 0x8A2C is the offset required
static unsigned char PlayerManagerPlayerListOffsetSig[] =
{
    0x8B, 0x35, 0x00, 0x00, 0x00, 0x00,
    0x8B, 0x86, 0x00, 0x00, 0x00, 0x00,
    0x2B, 0x86, 0x00, 0x00, 0x00, 0x00
};
const static MemSig PlayerManagerPlayerListOffset = {
    PlayerManagerPlayerListOffsetSig,
    "bb????bb????bbvv??"
};


#endif //MISSINGHUD2_REBIRTHMEMSIGNATURES_H
