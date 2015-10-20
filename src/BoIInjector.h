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

#ifndef BOISTATSREBORN_BOIINJECTOR_H
#define BOISTATSREBORN_BOIINJECTOR_H

#include <thread>
#include <cstdlib>

#include <Windows.h>
#include <easylogging++.h>

#include "BoIProcess.h"
#include "MHUD_MsgQueue.h"

struct InjectStatus
{
    enum Result
    {
        NOT_FOUND,
        FAIL,
        OK
    };

    InjectStatus() { }
    InjectStatus(Result cons_status) { inj_result = cons_status; }

    Result inj_result = Result::NOT_FOUND;
};
Q_DECLARE_METATYPE(InjectStatus);

class BoIInjector : public QObject
{
    Q_OBJECT

public:
    BoIInjector();
    ~BoIInjector();

    void Start();
    void Stop();

    inline BoIProcess* GetIsaacProcess()
    {
        return isaac_process_;
    }

signals:
    void InjectionStatus(InjectStatus s);
    void FatalError(std::string err_msg);

private:
    void InjectorThread();

    bool IsBoIRunning();
    void HandleDllMsgs();

    void LogFromDLL(mhud2::Log log_msg);

private:
    bool stop_injector_ = false;
    std::thread inject_thread_;
    BoIProcess *isaac_process_ = nullptr;
    MHUD::MsgQueue *dll_msg_queue_ = nullptr;
};


#endif //BOISTATSREBORN_BOIINJECTOR_H
