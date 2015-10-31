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

#ifndef MISSINGHUD2_MHUD_MSGQUEUE_H
#define MISSINGHUD2_MHUD_MSGQUEUE_H

#include <map>

#include <boost/interprocess/ipc/message_queue.hpp>

#include "mhud2.pb.h"
#include "MHUD_Options.h"

#define MAX_MSG_SIZE_BYTES          1024
#define MSG_QUEUE_APP_TO_DLL        "mhud2_app_to_dll"
#define MSG_QUEUE_DLL_TO_APP        "mhud2_dll_to_app"

#define QUEUE_LOG(LEVEL, MSG) MHUD::MsgQueue::GetInstance(MSG_QUEUE_DLL_TO_APP)->SendLog(LEVEL, MSG);
#define MHUD_IPC_LOG_MSG    0x1
#define MHUD_IPC_PREFS      0x2

typedef unsigned char byte;

namespace MHUD
{

struct MHUDMsg
{
    byte msg_type = 0;
    boost::interprocess::message_queue::size_type msg_size = 0;
    std::string msg_content = std::string(MAX_MSG_SIZE_BYTES, 0);
};

class MsgQueue
{
public:
    static MHUD::MsgQueue* GetInstance(std::string queue_name);
    static void Destroy(std::string queue_name);
    static void Remove(std::string queue_name);

    bool TryRecieve(MHUDMsg* mhud_msg);

    void SendLog(mhud2::Log::LogType log_type, std::string message);
    void SendPrefs(MHUD::Prefs mhud_prefs);

private:
    MsgQueue(std::string queue_name);
    ~MsgQueue();

private:
    static std::map<std::string,MHUD::MsgQueue*> instances_;

    boost::interprocess::message_queue *mhud2_msgs_ = nullptr;
};

}

#endif //MISSINGHUD2_MHUD_MSGQUEUE_H
