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

#ifndef MISSINGHUD2_RESOURCELOADER_H
#define MISSINGHUD2_RESOURCELOADER_H

#include <map>

#include <windows.h>

typedef int RESID;

struct FileResource
{
    FileResource() { };
    FileResource(LPVOID _bin_data, DWORD _res_size)
    {
        bin_data = _bin_data;
        res_size = _res_size;
    }

    LPVOID bin_data = nullptr;
    DWORD res_size = 0;
};

class ResourceLoader
{
public:
    static void Initialize(HMODULE hmodule);
    static void Destroy();

    static FileResource GetBinResource(RESID res_id);

private:
    ResourceLoader(HMODULE module);

    FileResource LoadBinResource(RESID res_id);

private:
    static ResourceLoader *instance_;

    HMODULE module_ = NULL;
    std::map<RESID, FileResource> loaded_resources_;
};

#endif //MISSINGHUD2_RESOURCELOADER_H
