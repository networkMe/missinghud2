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

#ifndef MISSINGHUD2_HUDSTAT_H
#define MISSINGHUD2_HUDSTAT_H

#include <vector>
#include <map>
#include <sstream>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <SOIL2.h>

#include "GLStructs.h"
#include "SpriteSheet.h"
#include "TextRenderer.h"
#include "ResourceLoader.h"
#include "ShaderProgram.h"
#include "DLLPreferences.h"
#include <res/DllResources.h>

#define SHOW_AS_PERCENTAGE  true
#define NO_RECENT_STAT_CHANGES_I   0
#define NO_RECENT_STAT_CHANGES_F   0.0

enum MHUDSTAT {
    kStat_Speed,
    kStat_Range,
    kStat_FireRate,
    kStat_ShotSpeed,
    kStat_ShotHeight,
    kStat_Damage,
    kStat_Luck,
    kStat_DealDoorChance,
    kStat_TearsFired,
    kStat_DealWithAngel,
    kStat_DealWithDevil
};

static const std::string MHUD2STAT_STRING[] = {
    "speed",
    "range",
    "firerate",
    "shotspeed",
    "shotheight",
    "damage",
    "luck",
    "deal_door_chance",
    "tears_fired",
    "deal_with_angel",
    "deal_with_devil"
};

class HUDStat
{
public:
    HUDStat(MHUDSTAT mhud_stat);
    ~HUDStat();

    void Draw(glm::vec2 position, float stat_value, float stat_change, bool percentage = false);
    void Draw(glm::vec2 position, int stat_value, int stat_change);

    std::string NumToStr(float number, bool percentage = false);
    std::string NumToStr(int number);

private:
    MHUDSTAT hud_stat_;
    Color positive_color_;
    Color negative_color_;
};


#endif //MISSINGHUD2_HUDSTAT_H
