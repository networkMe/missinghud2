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

#include "GDISwapBuffers.h"

GDISwapBuffers *GDISwapBuffers::instance_ = nullptr;

GDISwapBuffers::GDISwapBuffers()
{
}

GDISwapBuffers::~GDISwapBuffers()
{
}

GDISwapBuffers *GDISwapBuffers::GetInstance()
{
    if(!instance_)
        instance_ = new GDISwapBuffers();

    return instance_;
}

void GDISwapBuffers::Destroy()
{
    if (instance_)
        delete instance_;

    instance_ = nullptr;
}

void GDISwapBuffers::Cleanup()
{
    std::lock_guard<std::mutex> lock_guard(cleanup_mutex);

    if (cleanup_resources_ && !cleanup_complete_)
    {
        HUDOverlay::Destroy();
        ShaderProgram::DestroyAll();
        SpriteSheet::DestroyAllSpriteSheets();
        cleanup_complete_ = true;
        cleanup_complete_cv_.notify_all();
    }
}

void GDISwapBuffers::WaitForCleanup()
{
    std::unique_lock<std::mutex> uni_lock(cleanup_mutex);
    cleanup_resources_ = true;
    cleanup_complete_cv_.wait(uni_lock);
};

LPVOID GDISwapBuffers::GetGDI32HookAddr()
{
    if (orig_swap_buffers_addr_ == NULL)
    {
        HMODULE gdi32 = GetModuleHandle("gdi32.dll");
        orig_swap_buffers_addr_ = (LPVOID)GetProcAddress(gdi32, "SwapBuffers");
    }

    return orig_swap_buffers_addr_;
}

void GDISwapBuffers::CustomizeFrame(HDC hdc)
{
    // Initialize the GLEW OpenGL library on our first frame render (it requires a valid OpenGL context)
    if (!glew_ready_)
    {
        glewExperimental = GL_TRUE;
        glewInit();
        glew_ready_ = true;
    }

    // Disable DEPTH_TEST so we can draw over the top of Rebirth
    GLboolean orig_depthtest_val;
    glGetBooleanv(GL_DEPTH_TEST, &orig_depthtest_val);
    if (orig_depthtest_val)
        glDisable(GL_DEPTH_TEST);

    // Draw the HUD
    HUDOverlay *hud = HUDOverlay::GetInstance();
    hud->DrawHUD(hdc);

    // Re-enable DEPTH_TEST if it was enabled
    if (orig_depthtest_val)
        glEnable(GL_DEPTH_TEST);
}
