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

#include <boost/interprocess/ipc/message_queue.hpp>

#include "mhud2.pb.h"

#define MAX_MSG_SIZE_BYTES 1024
#define MSG_QUEUE_NAME "mhud2_msg_queue"

#define QUEUE_LOG(LEVEL, MSG) MHUD::MsgQueue::GetInstance()->SendLog(LEVEL, MSG);
#define MHUD_IPC_LOG_MSG 0x1

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
    static MsgQueue* GetInstance();
    static void Destroy();
    static void Remove();

    bool TryRecieve(MHUDMsg* mhud_msg);

    void SendLog(mhud2::Log::LogType log_type, std::string message);

private:
    MsgQueue();
    ~MsgQueue();

private:
    static MsgQueue* instance_;

    boost::interprocess::message_queue *mhud2_msgs_ = nullptr;
};

}

#endif //MISSINGHUD2_MHUD_MSGQUEUE_H
