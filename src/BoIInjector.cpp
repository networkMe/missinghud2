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
    : inject_thread_()
{
    isaac_process_ = BoIProcess::GetInstance();
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
    while (!stop_injector_)
    {
        if (IsBoIRunning())
        {
            LOG(INFO) << "BoI process found, time to inject...";

            isaac_process_ = BoIProcess::GetInstance();
            try
            {
                isaac_process_->HookBoIProcess();
            }
            catch (std::runtime_error &e)
            {
                LOG(ERROR) << "BoI process injection failed: " << e.what();
                emit FatalError(e.what());
                return;
            }

            // Notify that we're injected
            LOG(INFO) << "BoI process injected successfully.";
            emit InjectionStatus(InjectStatus(InjectStatus::Result::OK));

            // Just sleep until Isaac closes or injector is quitting
            while (isaac_process_->IsRunning() && !stop_injector_)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            // Eject our DLLs (if they are still in the process)
            LOG(INFO) << "BoI process closed or MissingHUD2 requested quit. Cleanup initiated.";
            BoIProcess::Close();
        }
        else
        {
            emit InjectionStatus(InjectStatus(InjectStatus::Result::NOT_FOUND));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

bool BoIInjector::IsBoIRunning()
{
    return (FindWindow(NULL, "Binding of Isaac: Rebirth") != NULL);
}
