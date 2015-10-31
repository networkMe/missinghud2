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

#ifndef MISSINGHUD2_DLLPREFERENCES_H
#define MISSINGHUD2_DLLPREFERENCES_H

#include <thread>
#include <chrono>

#include "../MHUD_Options.h"
#include "../MHUD_MsgQueue.h"

class DLLPreferences
{
public:
    static DLLPreferences * GetInstance();
    static void Destroy();

    MHUD::Prefs GetPrefs()
    {
        return current_prefs_;
    };

private:
    DLLPreferences();
    ~DLLPreferences();

    void MsgMonitor();

private:
    static DLLPreferences *instance_;

    bool quit_monitoring_ = false;
    std::thread monitor_thread_;
    MHUD::Prefs current_prefs_;
};

#endif //MISSINGHUD2_DLLPREFERENCES_H
