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

#ifndef MISSINGHUD2_IATHOOK_H
#define MISSINGHUD2_IATHOOK_H

#include <string>
#include <map>

#include <windows.h>
#include <winnt.h>

struct IATHookInfo
{
    bool hook_enabled = false;

    HMODULE app_module = 0;
    std::string target_module;
    std::string proc_name;
    LPVOID detour_proc = nullptr;
    LPVOID orig_proc_addr = nullptr;

    IMAGE_THUNK_DATA *thunk = nullptr;
};

class IATHook
{
public:
    static bool InitIATHook(HMODULE app_module, std::string target_module, std::string proc_name, LPVOID detour_proc);

    static bool EnableIATHook(std::string proc_name, LPVOID *orig_proc_addr);
    static bool DisableIATHook(std::string proc_name);

private:
    static std::map<std::string, IATHookInfo> iat_hooks_;
};

#endif //MISSINGHUD2_IATHOOK_H
