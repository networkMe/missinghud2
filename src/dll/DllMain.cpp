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
#include <MinHook.h>

#include "RebirthMemReader.h"
#include "GDISwapBuffers.h"
#include "ResourceLoader.h"

#define DLL_PUBLIC __declspec(dllexport)

static HMODULE dll_handle = 0;

BOOL WINAPI gdiSwapBuffersDetour(HDC hdc);

int APIENTRY DllMain(HMODULE h_dll, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
        dll_handle = h_dll;

    return TRUE;
}

extern "C" DLL_PUBLIC void HUD2_Start()
{
    OutputDebugString("[HUD2_Start] MissingHUD2 injected and starting.");

    // Initialize MinHook library
    if (MH_Initialize() != MH_OK)
        FreeLibraryAndExitThread(dll_handle, EXIT_FAILURE);

    // Initialize any static objects we require that don't involve an OpenGL context
    ResourceLoader::Initialize(dll_handle);
    RebirthMemReader::GetMemoryReader();

    // Hook the OpenGL SwapBuffers function
    GDISwapBuffers *gdi_swapbuffers = GDISwapBuffers::GetInstance();
    if (MH_CreateHook(gdi_swapbuffers->GetGDI32HookAddr(), (LPVOID)&gdiSwapBuffersDetour, (LPVOID*) gdi_swapbuffers->GetEndpointAddr()) != MH_OK)
        FreeLibraryAndExitThread(dll_handle, EXIT_FAILURE);

    // Enable the hooks
    if (MH_EnableHook(gdi_swapbuffers->GetGDI32HookAddr()) != MH_OK)
        FreeLibraryAndExitThread(dll_handle, EXIT_FAILURE);
}

extern "C" DLL_PUBLIC void HUD2_Stop()
{
    OutputDebugString("[HUD2_Stop] MissingHUD2 exiting.");

    // Tell our SwapBuffers detour to cleanup it's OpenGL stuff
    GDISwapBuffers *gdi_swapbuffers = GDISwapBuffers::GetInstance();
    gdi_swapbuffers->FlagCleanup();

    // Wait for the cleanup to happen in the OpenGL context thread
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Disable MinHook hooks
    MH_DisableHook(gdi_swapbuffers->GetGDI32HookAddr());

    // Wait for the hooks to be no longer active
    // aka. for Rebirth to exit our detours for the last time
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Clean-up global static objects
    GDISwapBuffers::Destroy();
    ResourceLoader::Destroy();
    RebirthMemReader::Destroy();

    // Destroy MinHook and exit our DLL, Rebirth should be a lot less informative now!
    MH_Uninitialize();
    FreeLibraryAndExitThread(dll_handle, EXIT_SUCCESS);
}

BOOL WINAPI gdiSwapBuffersDetour(HDC hdc)
{
    GDISwapBuffers *gdi_swapbuffers = GDISwapBuffers::GetInstance();

    // Cleanup needs to be called from within the thread with the GL context
    // It cleans up many resources that require OpenGL access (glDeleteXXXXXX)
    if (gdi_swapbuffers->ShouldCleanup())
    {
        gdi_swapbuffers->Cleanup();
    }
    else if (hdc != NULL) // NULL hdc is illegal (according to Microsoft)
    {
        gdi_swapbuffers->CustomizeFrame(hdc);
    }

    // Pass control back off to the original SwapBuffers Win32 function (and thus Rebirth...)
    return (*gdi_swapbuffers->GetEndpointAddr())(hdc);
}
