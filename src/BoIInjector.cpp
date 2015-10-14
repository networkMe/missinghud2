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

#include "BoIInjector.h"

BoIInjector::BoIInjector()
{
}

BoIInjector::~BoIInjector()
{
    Stop();
}

void BoIInjector::Start()
{
    LOG(INFO) << L"Starting DLL injection monitor thread.";
    inject_thread_ = std::thread(&BoIInjector::InjectorThread, this);
}

void BoIInjector::Stop()
{
    LOG(INFO) << L"Stopping DLL injection monitor thread.";
    stop_injector_ = true;
    if (inject_thread_.joinable())
        inject_thread_.join();
}

void BoIInjector::InjectorThread()
{
    try
    {
        while (!stop_injector_)
        {
            if (IsBoIRunning())
            {
                LOG(INFO) << L"BoI process found, time to inject...";

                // Set-up the DLL message queue (for IPC)
                MHUD::MsgQueue::Remove();
                dll_msg_queue_ = MHUD::MsgQueue::GetInstance();
                std::thread handle_dll_msgs = std::thread(&BoIInjector::HandleDllMsgs, this);
                handle_dll_msgs.detach();

                // Inject DLL
                isaac_process_ = BoIProcess::GetInstance();
                isaac_process_->HookBoIProcess();

                // Notify that we're injected
                LOG(INFO) << L"BoI process injected successfully.";
                emit InjectionStatus(InjectStatus(InjectStatus::Result::OK));

                // Just sleep until Isaac closes or injector is quitting
                while (isaac_process_->IsRunning() && !stop_injector_)
                {
                    if (!isaac_process_->MHUD2Active())  // MHUD2 should not unload until we want it to (or Isaac closes)
                        throw MHUD_Error(L"Error occurred within injected DLL. Check MHUD2.log for details.");

                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }

                // Eject our DLLs (if they are still in the running BoI process)
                LOG(INFO) << L"BoI process closed or MissingHUD2 requested quit.";
                BoIProcess::Close();

                // Clean-up DLL message queue
                dll_msg_queue_ = nullptr;
                MHUD::MsgQueue::Destroy();
            }
            else
            {
                emit InjectionStatus(InjectStatus(InjectStatus::Result::NOT_FOUND));
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    catch (MHUD_Error &e)
    {
        LOG(ERROR) << L"Injection thread error occured: " << e.get_error() << L" (Error Code: " << GetLastError() << L")";
        emit FatalError(e.get_error());
    }
    catch (boost::interprocess::interprocess_exception &ie)
    {
        LOG(ERROR) << L"Interprocess communication error occured: " << ie.what() << L" (Error Code: " << GetLastError() << L")";
        emit FatalError(L"Interprocess communication error occured. Check MHUD2.log for details.");
    }
    catch (...)
    {
        LOG(ERROR) << L"Unknown error occured. " << L" (Error Code: " << GetLastError() << L")";
        emit FatalError(L"Unknown error occured.");
    }
}

bool BoIInjector::IsBoIRunning()
{
    bool isaac_running = false;

    PROCESSENTRY32 proc_entry = { 0 };
    proc_entry.dwSize = sizeof(proc_entry);
    HANDLE proc_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (proc_snapshot == INVALID_HANDLE_VALUE)
        throw MHUD_Error(L"[IsBoIRunning] Unable to obtain the system's process list to check if Isaac is running.");
    if (!Process32First(proc_snapshot, &proc_entry))
        throw MHUD_Error(L"[IsBoIRunning] Unable to read the system's process list to check if Isaac is running.");
    do
    {
        if (std::wstring(proc_entry.szExeFile) == BOI_PROCESS_NAME)
        {
            isaac_running = true;
            break;
        }
    } while (Process32Next(proc_snapshot, &proc_entry));
    CloseHandle(proc_snapshot);

    return isaac_running;
}

void BoIInjector::HandleDllMsgs()
{
    try
    {
        while (dll_msg_queue_ != nullptr)
        {
            MHUD::MHUDMsg mhud_msg;
            while(MHUD::MsgQueue::GetInstance()->TryRecieve(&mhud_msg))
            {
                switch(mhud_msg.msg_type)
                {
                    case MHUD_IPC_LOG_MSG:
                    {
                        mhud2::Log log_msg;
                        log_msg.ParseFromString(mhud_msg.msg_content);
                        LogFromDLL(log_msg);
                    } break;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    catch(boost::interprocess::interprocess_exception &ie)
    {
        LOG(ERROR) << L"Boost Interprocess error occurred: " << ie.what();
        emit FatalError(L"Boost Interprocess error occurred in DLL interprocess message thread.");
    }
    catch (...)
    {
        LOG(ERROR) << L"Unknown error occurred in DLL interprocess message thread.";
        emit FatalError(L"Unknown error occurred in DLL interprocess message thread.");
    }
}

void BoIInjector::LogFromDLL(mhud2::Log log_msg)
{
    if (log_msg.has_log_type() && log_msg.has_log_msg())
    {
        switch (log_msg.log_type())
        {
            case log_msg.LOG_INFO:
            {
                LOG(INFO) << "[MHUD2 DLL] " << log_msg.log_msg();
            } break;

            case log_msg.LOG_ERROR:
            {
                LOG(ERROR) << "[MHUD2 DLL] " << log_msg.log_msg();
            } break;
        }
    }
}
