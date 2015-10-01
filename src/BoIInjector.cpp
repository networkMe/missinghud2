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
    LOG(INFO) << "Starting DLL injection monitor thread.";
    inject_thread_ = std::thread(&BoIInjector::InjectorThread, this);
}

void BoIInjector::Stop()
{
    LOG(INFO) << "Stopping DLL injection monitor thread.";
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
                LOG(INFO) << "BoI process found, time to inject...";

                isaac_process_ = BoIProcess::GetInstance();
                isaac_process_->HookBoIProcess();

                // Notify that we're injected
                LOG(INFO) << "BoI process injected successfully.";
                emit InjectionStatus(InjectStatus(InjectStatus::Result::OK));

                // Just sleep until Isaac closes or injector is quitting
                while (isaac_process_->IsRunning() && !stop_injector_)
                {
                    if (!isaac_process_->MHUD2Active())  // MHUD2 should not unload until we want it to (or Isaac closes)
                        throw std::runtime_error("Error occured within injected DLL. Check MHUD2.log for details.");

                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }

                // Eject our DLLs (if they are still in the running BoI process)
                LOG(INFO) << "BoI process closed or MissingHUD2 requested quit.";
                BoIProcess::Close();
            }
            else
            {
                emit InjectionStatus(InjectStatus(InjectStatus::Result::NOT_FOUND));
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    catch (std::runtime_error &e)
    {
        LOG(ERROR) << "Injection thread error occured: " << e.what() << " (Error Code: " << GetLastError() << ")";
        emit FatalError(e.what());
    }
    catch (...)
    {
        LOG(ERROR) << "Unknown error occured. " << " (Error Code: " << GetLastError() << ")";
        emit FatalError("Unknown error occured.");
    }
}

bool BoIInjector::IsBoIRunning()
{
    bool isaac_running = false;

    PROCESSENTRY32 proc_entry = { 0 };
    proc_entry.dwSize = sizeof(proc_entry);
    HANDLE proc_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (proc_snapshot == INVALID_HANDLE_VALUE)
        throw std::runtime_error("[IsBoIRunning] Unable to obtain the system's process list to check if Isaac is running.");
    if (!Process32First(proc_snapshot, &proc_entry))
        throw std::runtime_error("[IsBoIRunning] Unable to read the system's process list to check if Isaac is running.");
    do
    {
        if (std::string(proc_entry.szExeFile) == BOI_PROCESS_NAME)
        {
            isaac_running = true;
            break;
        }
    } while (Process32Next(proc_snapshot, &proc_entry));
    CloseHandle(proc_snapshot);

    return isaac_running;
}
