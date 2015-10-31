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

#include <thread>

#include <windows.h>

#include "IATHook.h"
#include "RebirthMemReader.h"
#include "GDISwapBuffers.h"
#include "ResourceLoader.h"
#include "DLLPreferences.h"

#define DLL_PUBLIC __declspec(dllexport)

static HMODULE dll_handle = 0;

BOOL WINAPI gdiSwapBuffersDetour(HDC hdc);

int APIENTRY DllMain(HMODULE h_dll, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
        dll_handle = h_dll;

    return TRUE;
}

extern "C" DLL_PUBLIC void MHUD2_Start()
{
    try
    {
        QUEUE_LOG(mhud2::Log::LOG_INFO, "MissingHUD2 injected and starting.");

        // Initialize any static objects we require that don't involve an OpenGL context
        DLLPreferences::GetInstance();
        ResourceLoader::Initialize(dll_handle);
        RebirthMemReader::GetMemoryReader();

        // Hook the OpenGL SwapBuffers function via IAT redirection
        GDISwapBuffers *gdi_swapbuffers = GDISwapBuffers::GetInstance();
        IATHook::InitIATHook(GetModuleHandleW(WCHAR_ISAAC_MODULE_NAME), "gdi32.dll", "SwapBuffers",
                             (LPVOID)&gdiSwapBuffersDetour);

        // Enable the IAT hooks
        IATHook::EnableIATHook("SwapBuffers", (LPVOID*)gdi_swapbuffers->GetEndpointAddr());
    }
    catch (std::runtime_error &e)
    {
        QUEUE_LOG(mhud2::Log::LOG_ERROR, "Error occured during MHUD2 initilization: " + std::string(e.what()));
        FreeLibraryAndExitThread(dll_handle, EXIT_FAILURE);
    }
    catch (boost::interprocess::interprocess_exception &ie)
    {
        FreeLibraryAndExitThread(dll_handle, EXIT_FAILURE);
    }
}

extern "C" DLL_PUBLIC void MHUD2_Stop()
{
    QUEUE_LOG(mhud2::Log::LOG_INFO, "MissingHUD2 exiting.");

    // Tell our SwapBuffers detour to cleanup it's OpenGL stuff
    GDISwapBuffers *gdi_swapbuffers = GDISwapBuffers::GetInstance();
    gdi_swapbuffers->WaitForCleanup();

    try
    {
        // Disable IAT hooks
        IATHook::DisableIATHook("SwapBuffers");

        // Wait for the hooks to be no longer active
        // aka. for Rebirth to exit our detours for the last time
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Clean-up global static objects
        DLLPreferences::Destroy();
        GDISwapBuffers::Destroy();
        ResourceLoader::Destroy();
        RebirthMemReader::Destroy();
        MHUD::MsgQueue::Destroy(MSG_QUEUE_DLL_TO_APP);
    }
    catch (std::runtime_error &e)
    {
        QUEUE_LOG(mhud2::Log::LOG_ERROR, "Error occured during MHUD2 cleanup: " + std::string(e.what()));
        FreeLibraryAndExitThread(dll_handle, EXIT_FAILURE);
    }

    // Exit our DLL, Rebirth should be a lot less informative now!
    FreeLibraryAndExitThread(dll_handle, EXIT_SUCCESS);
}

static bool frame_failed = false;
BOOL WINAPI gdiSwapBuffersDetour(HDC hdc)
{
    GDISwapBuffers *gdi_swapbuffers = GDISwapBuffers::GetInstance();

    // Cleanup needs to be called from within the thread with the GL context
    // It cleans up many resources that require OpenGL access (glDeleteXXXXXX)
    if (gdi_swapbuffers->ShouldCleanup())
    {
        gdi_swapbuffers->Cleanup();
    }
    else if (!frame_failed && hdc != NULL) // NULL hdc is illegal (according to Microsoft)
    {
        if (!gdi_swapbuffers->CustomizeFrame(hdc))
        {
            frame_failed = true;

            // We use the raw Win32 API to create the stop thread as using the C++ variant (std::thread) results
            // in Rebirth crashing (this isn't our programs thread)
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&MHUD2_Stop, NULL, 0, NULL);
        }
    }

    // Pass control back off to the original SwapBuffers Win32 function (and thus Rebirth...)
    return (*gdi_swapbuffers->GetEndpointAddr())(hdc);
}
