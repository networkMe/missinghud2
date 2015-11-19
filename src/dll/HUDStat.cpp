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

    Color green(0, 200, 0);
    Color red(220, 0, 0);
    switch (mhud_stat)
    {
        case MHUDSTAT::kStat_FireRate:
        {
            negative_color_ = green;
            positive_color_ = red;
        } break;

        default:
        {
            positive_color_ = green;
            negative_color_ = red;
        }
    }
}

HUDStat::~HUDStat()
{
}

void HUDStat::Draw(glm::vec2 position, float stat_value, float stat_change, bool percentage)
{
    // Draw icon sprite
    glm::vec2 icon_size(32.0f * HUDOverlay::GetInstance()->GetHUDSizeMultiplier(), 32.0f * HUDOverlay::GetInstance()->GetHUDSizeMultiplier());
    SpriteSheet *icon_sprite = SpriteSheet::GetSpriteSheet(MHUD2_STAT_ICONS);
    icon_sprite->DrawSprite(position, icon_size, MHUD2STAT_STRING[hud_stat_]);

    // Draw statistic value
    position.x += 32.0f * HUDOverlay::GetInstance()->GetHUDSizeMultiplier();
    position.y -= 8.0f * HUDOverlay::GetInstance()->GetHUDSizeMultiplier();
    TextRenderer *isaac_text = TextRenderer::GetRenderer(MHUD2_ISAAC_FONT_PNG, MHUD2_ISAAC_FONT_CHARMAP);
    glm::vec2 text_render_size = isaac_text->RenderText(position, NumToStr(stat_value, percentage), Color(255, 255, 255));

    // Draw the change in statistic value if it recently changed
    position.x += text_render_size.x;
    if (stat_change > 0.0f)
        isaac_text->RenderText(position, "+" + NumToStr(stat_change, percentage), positive_color_);
    else if (stat_change < 0.0f)
        isaac_text->RenderText(position, NumToStr(stat_change, percentage), negative_color_);  // Negative symbol automatic via C++
}

void HUDStat::Draw(glm::vec2 position, int stat_value, int stat_change)
{
    // Draw icon sprite
    glm::vec2 icon_size(32.0f * HUDOverlay::GetInstance()->GetHUDSizeMultiplier(), 32.0f * HUDOverlay::GetInstance()->GetHUDSizeMultiplier());
    SpriteSheet *icon_sprite = SpriteSheet::GetSpriteSheet(MHUD2_STAT_ICONS);
    icon_sprite->DrawSprite(position, icon_size, MHUD2STAT_STRING[hud_stat_]);

    // Draw statistic value
    position.x += 32.0f * HUDOverlay::GetInstance()->GetHUDSizeMultiplier();
    position.y -= 8.0f * HUDOverlay::GetInstance()->GetHUDSizeMultiplier();
    TextRenderer *isaac_text = TextRenderer::GetRenderer(MHUD2_ISAAC_FONT_PNG, MHUD2_ISAAC_FONT_CHARMAP);
    glm::vec2 text_render_size = isaac_text->RenderText(position, NumToStr(stat_value), Color(255, 255, 255));

    // Draw the change in statistic value if it recently changed
    position.x += text_render_size.x;
    if (stat_change > 0)
        isaac_text->RenderText(position, "+" + NumToStr(stat_change), positive_color_);
    else if (stat_change < 0)
        isaac_text->RenderText(position, NumToStr(stat_change), negative_color_); // Negative symbol automatic via C++
}

std::string HUDStat::NumToStr(float number, bool percentage)
{
    std::stringstream ss;

    if (percentage)
    {
        if ((float)number != (int)number) // float has decimal part
        {
            ss.setf(std::ios::fixed, std::ios::floatfield);
            ss.precision(DLLPreferences::GetInstance()->GetPrefs().stat_precision);
        }

        ss << (number * 100) << "%";
    }
    else
    {
        ss.setf(std::ios::fixed, std::ios::floatfield);
        ss.precision(DLLPreferences::GetInstance()->GetPrefs().stat_precision);
        ss << number;
    }

    return ss.str();
}

std::string HUDStat::NumToStr(int number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}
