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

#include "DLLPreferences.h"

DLLPreferences *DLLPreferences::instance_ = nullptr;

DLLPreferences *DLLPreferences::GetInstance()
{
    if (instance_ == nullptr)
        instance_ = new DLLPreferences();

    return instance_;
}

void DLLPreferences::Destroy()
{
    if (instance_ != nullptr)
        delete instance_;

    instance_ = nullptr;
}

DLLPreferences::DLLPreferences()
{
    monitor_thread_ = std::thread(MsgMonitor, this);
}

DLLPreferences::~DLLPreferences()
{
    quit_monitoring_ = true;
    if (monitor_thread_.joinable())
        monitor_thread_.join();
}

void DLLPreferences::MsgMonitor()
{
    while (!quit_monitoring_)
    {
        MHUD::MHUDMsg mhud_msg;
        while (MHUD::MsgQueue::GetInstance(MSG_QUEUE_APP_TO_DLL)->TryRecieve(&mhud_msg))
        {
            switch (mhud_msg.msg_type)
            {
                case MHUD_IPC_PREFS:
                {
                    mhud2::Preferences prefs_proto;
                    prefs_proto.ParseFromString(mhud_msg.msg_content);

                    current_prefs_.show_tears_fired = prefs_proto.show_tears_fired();
                    current_prefs_.show_shot_height = prefs_proto.show_shot_height();
                    current_prefs_.stat_precision = prefs_proto.stat_precision();
                } break;
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
