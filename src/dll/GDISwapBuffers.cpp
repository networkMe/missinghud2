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
    cleanup_complete_cv_.wait_for(uni_lock, std::chrono::seconds(5));
};

LPVOID GDISwapBuffers::GetGDI32HookAddr()
{
    if (orig_swap_buffers_addr_ == NULL)
    {
        HMODULE gdi32 = GetModuleHandleW(L"gdi32.dll");
        orig_swap_buffers_addr_ = (LPVOID)GetProcAddress(gdi32, "SwapBuffers");
    }

    return orig_swap_buffers_addr_;
}

bool GDISwapBuffers::CustomizeFrame(HDC hdc)
{
    // Initialize the GLEW OpenGL library on our first frame render (it requires a valid OpenGL context)
    if (!glew_ready_)
    {
        if (!InitializeGLEW())
            return false;
    }

    // Disable DEPTH_TEST so we can draw over the top of Rebirth
    GLboolean orig_depthtest_val;
    glGetBooleanv(GL_DEPTH_TEST, &orig_depthtest_val);
    if (orig_depthtest_val)
        glDisable(GL_DEPTH_TEST);

    // Enable BLEND so that we can use partially transparent PNG's
    GLboolean orig_blend_val;
    glGetBooleanv(GL_BLEND, &orig_blend_val);
    if (!orig_blend_val)
        glEnable (GL_BLEND);

    // Set our OpenGL BlendFunc for alpha transparency
    GLint orig_blend_src;
    glGetIntegerv(GL_BLEND_SRC, &orig_blend_src);
    GLint orig_blend_dst;
    glGetIntegerv(GL_BLEND_DST, &orig_blend_dst);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw the HUD
    HUDOverlay *hud = HUDOverlay::GetInstance();
    hud->DrawHUD(hdc);

    // Restore whatever the original BlendFunc was
    glBlendFunc(orig_blend_src, orig_blend_dst);

    // Disable BLEND if it was disabled
    if (!orig_blend_val)
        glDisable(GL_BLEND);

    // Re-enable DEPTH_TEST if it was enabled
    if (orig_depthtest_val)
        glEnable(GL_DEPTH_TEST);

    return true;
}

bool GDISwapBuffers::InitializeGLEW()
{
    if (!glew_ready_)
    {
        glewExperimental = GL_TRUE;
        GLenum glew_result = glewInit();
        if (glew_result != GLEW_OK)
        {
            std::stringstream ss;
            ss << "glewInit failed with error: " << glew_result << " (" << glewGetErrorString(glew_result) << ")";
            QUEUE_LOG(mhud2::Log::LOG_ERROR, ss.str());

            return false;
        }

        // MHUD2 requires minimum OpenGL 2.0 support (it was released mid 2004)
        if (!GLEW_VERSION_2_0)
        {
            QUEUE_LOG(mhud2::Log::LOG_ERROR, "MHUD2 requires OpenGL 2.0 graphics driver support.");
            return false;
        }

        glew_ready_ = true;
    }

    return true;
}
