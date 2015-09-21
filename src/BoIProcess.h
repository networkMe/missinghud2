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

#ifndef BOISTATSREBORN_BOIPROCESS_H
#define BOISTATSREBORN_BOIPROCESS_H

#include <iostream>
#include <stdexcept>
#include <functional>

#include <QCoreApplication>
#include <easylogging++.h>

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <shlwapi.h>

#define BOI_PROCESS_NAME    "isaac-ng.exe"

class BoIProcess
{
public:
    static BoIProcess* GetInstance();
    static void Close();

    bool HookBoIProcess();
    bool IsRunning();

private:
    BoIProcess();
    ~BoIProcess();

    HMODULE LoadLibraryIntoBoI(LPVOID remote_mem, std::string library_path);
    FARPROC GetRemoteProcAddress(HMODULE rem_dll_module, std::string rem_module_name, std::string proc_name);
    bool UnhookBoIProcess();

private:
    static BoIProcess* instance_;

    HANDLE process_ = NULL;
    HMODULE injected_dll_ = NULL;
};

#endif //BOISTATSREBORN_BOIPROCESS_H
