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

#ifndef MISSINGHUD2_GLSWAPBUFFERS_H
#define MISSINGHUD2_GLSWAPBUFFERS_H

#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>

#include <windows.h>
#include <GL/glew.h>

#include "GLStructs.h"
#include "HUDOverlay.h"
#include <res/DllResources.h>

typedef BOOL WINAPI (*GDISWAPBUFFERSFUNC)(HDC);

class GDISwapBuffers
{
public:
    static GDISwapBuffers *GetInstance();
    static void Destroy();

    void CustomizeFrame(HDC hdc);

    void Cleanup();
    void WaitForCleanup();
    inline bool ShouldCleanup()
    {
        std::lock_guard<std::mutex> lock_guard(cleanup_mutex);
        return cleanup_resources_;
    };

    LPVOID GetGDI32HookAddr();
    inline GDISWAPBUFFERSFUNC *GetEndpointAddr()
    {
        return &endpoint_addr_;
    };

private:
    GDISwapBuffers();
    ~GDISwapBuffers();

private:
    static GDISwapBuffers *instance_;

    std::mutex cleanup_mutex;
    bool cleanup_resources_ = false;
    bool cleanup_complete_ = false;
    std::condition_variable cleanup_complete_cv_;

    bool glew_ready_ = false;

    LPVOID orig_swap_buffers_addr_ = NULL;
    GDISWAPBUFFERSFUNC endpoint_addr_ = nullptr;
};


#endif //MISSINGHUD2_GLSWAPBUFFERS_H
