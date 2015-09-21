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

#include "HUDStat.h"

HUDStat::HUDStat(MHUDSTAT mhud_stat)
{
    hud_stat_ = mhud_stat;
}

HUDStat::~HUDStat()
{
}

void HUDStat::Draw(glm::vec2 position, float stat_value)
{
    // Draw icon sprite
    glm::vec2 icon_size(0.08f, 0.13f);
    SpriteSheet *icon_sprite = SpriteSheet::GetSpriteSheet(MHUD2_STAT_ICONS);
    icon_sprite->DrawSprite(position, icon_size, MHUD2STAT_STRING[hud_stat_]);

    // Get value as string
    std::stringstream val_stream;
    val_stream.setf(std::ios::fixed, std::ios::floatfield);
    val_stream.precision(1);
    val_stream << stat_value;

    // Draw statistic value
    position.x += 0.075f;
    position.y -= 0.0375f;
    TextRenderer *isaac_text = TextRenderer::GetRenderer(MHUD2_ISAAC_FONT_PNG, MHUD2_ISAAC_FONT_CHARMAP);
    isaac_text->RenderText(position, val_stream.str());
}
