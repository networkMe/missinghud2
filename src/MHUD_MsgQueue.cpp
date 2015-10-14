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

#include "MHUD_MsgQueue.h"

using namespace boost::interprocess;

MHUD::MsgQueue* MHUD::MsgQueue::instance_ = nullptr;

MHUD::MsgQueue *MHUD::MsgQueue::GetInstance()
{
    if (instance_ == nullptr)
        instance_ = new MsgQueue();

    return instance_;
}

void MHUD::MsgQueue::Destroy()
{
    if (instance_ != nullptr)
        delete instance_;

    instance_ = nullptr;
}

void MHUD::MsgQueue::Remove()
{
    if (instance_ != nullptr)
        Destroy();

    message_queue::remove(MSG_QUEUE_NAME);
}

MHUD::MsgQueue::MsgQueue()
{
    // Set-up the Dll message queue (for IPC)
    mhud2_msgs_ = new message_queue(open_or_create, MSG_QUEUE_NAME, 50, sizeof(byte) * (MAX_MSG_SIZE_BYTES + 1));
}

MHUD::MsgQueue::~MsgQueue()
{
    delete mhud2_msgs_;
}

bool MHUD::MsgQueue::TryRecieve(MHUD::MHUDMsg *mhud_msg)
{
    unsigned int msg_prio = 0;
    message_queue::size_type msg_size = 0;
    std::string msg_buffer(MAX_MSG_SIZE_BYTES + 1, 0);
    if (!mhud2_msgs_->try_receive(&msg_buffer[0], MAX_MSG_SIZE_BYTES + 1, msg_size, msg_prio))
        return false;

    // Get message type
    byte message_type = msg_buffer[0];
    msg_buffer.erase(0, 1);

    // Parse message
    switch(message_type)
    {
        case MHUD_IPC_LOG_MSG:
        {
            mhud_msg->msg_type = MHUD_IPC_LOG_MSG;
            mhud_msg->msg_size = msg_size;
            mhud_msg->msg_content = msg_buffer;
        } break;

        default:
        {
            mhud_msg->msg_type = 255;
            mhud_msg->msg_size = 0;
            mhud_msg->msg_content = "";
        } break;
    }

    return true;
}

void MHUD::MsgQueue::SendLog(mhud2::Log::LogType log_type, std::string message)
{
    mhud2::Log log_proto;
    log_proto.set_log_type(log_type);
    log_proto.set_log_msg(message);

    unsigned int msg_prio = 0;
    std::string msg_buffer(MAX_MSG_SIZE_BYTES, 0);
    log_proto.SerializeToString(&msg_buffer);
    msg_buffer.insert(0, 1, MHUD_IPC_LOG_MSG);

    mhud2_msgs_->send(&msg_buffer[0], MAX_MSG_SIZE_BYTES + 1, msg_prio);
}