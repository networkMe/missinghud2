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



// 8B 1D B4 C4 2D 01        | mov ebx,dword ptr ds:[<&timeGetTime>]         |
// C7 05 6C E4 33 01 40 5D  | mov dword ptr ds:[133E46C],isaac-ng.1275D40   |
// C7 05 70 E4 33 01 D0 5D  | mov dword ptr ds:[133E470],isaac-ng.1275DD0   |
// C7 05 74 E4 33 01 10 5E  | mov dword ptr ds:[133E474],isaac-ng.1275E10   |
// FF D3                    | call ebx                                      |
// A3 F0 A1 33 01           | mov dword ptr ds:[133A1F0],eax                |  <== 0x133A1F0 is the address required
const static unsigned char PlayerManagerInstAddrSig[] =
{
    0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00,
    0xC7, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xC7, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xC7, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xD3,
    0xA3, 0x00, 0x00, 0x00, 0x00
};
const static MemSig PlayerManagerInstAddr = {
    PlayerManagerInstAddrSig,
    "bb????bb????????bb????????bb????????bbbvvvv"
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
