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

#ifndef MISSINGHUD2_HUDOVERLAY_H
#define MISSINGHUD2_HUDOVERLAY_H

#include <windows.h>

#include <GL/glew.h>
#include <SOIL2.h>

#include "GLStructs.h"
#include "TextRenderer.h"
#include "HUDStat.h"
#include "MemReader.h"

class HUDOverlay
{
public:
    static HUDOverlay *GetInstance();
    static void Destroy();

    void DrawHUD(HDC hdc);

    VIEWSIZE GetViewportSize();
    float GetHUDSizeMultiplier();

private:
    HUDOverlay();
    ~HUDOverlay();

private:
    static HUDOverlay *instance_;

    VIEWSIZE viewport_size_;
    std::chrono::time_point<std::chrono::system_clock> viewport_updated_ = std::chrono::system_clock::now();
};

#endif //MISSINGHUD2_HUDOVERLAY_H
