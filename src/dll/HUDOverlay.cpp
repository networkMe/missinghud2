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

#include "HUDOverlay.h"

HUDOverlay *HUDOverlay::instance_ = nullptr;

HUDOverlay *HUDOverlay::GetInstance()
{
    if (instance_ == nullptr)
        instance_ = new HUDOverlay();

    return instance_;
}

void HUDOverlay::Destroy()
{
    if (instance_)
        delete instance_;

    instance_ = nullptr;
}

HUDOverlay::HUDOverlay()
{
    SpriteSheet::LoadSpriteSheet(MHUD2_STAT_ICONS, MHUD2_STAT_ICONS_TEXMAP, SPRITESIZE(32, 32), true);
}

HUDOverlay::~HUDOverlay()
{
    // We don't need to text render if we have no overlay
    TextRenderer::DestroyAll();

    // No need for the below spritesheets either
    SpriteSheet::DestroySpriteSheet(MHUD2_STAT_ICONS);
}

void HUDOverlay::DrawHUD(HDC hdc)
{
    // Get the Rebirth memory reader to get the HUD stat values
    RebirthMemReader *mem_reader = RebirthMemReader::GetMemoryReader();
    if (!mem_reader->IsRunActive())
        return; // We don't want to draw the HUD if the player isn't in a Rebirth run

    // Draw the HUD stats info bar
    glm::vec2 base_hud_stats_menu;
    base_hud_stats_menu.x = -0.98f;
    base_hud_stats_menu.y = 0.1f;

    HUDStat speed_stat(MHUDSTAT::kStat_Speed);
    speed_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStatf(RebirthPlayerStat::kSpeed),
                    mem_reader->GetPlayerRecentStatChangef(RebirthPlayerStat::kSpeed));

    base_hud_stats_menu.y -= 0.1f;
    HUDStat range_stat(MHUDSTAT::kStat_Range);
    range_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStatf(RebirthPlayerStat::kRange),
                    mem_reader->GetPlayerRecentStatChangef(RebirthPlayerStat::kRange));

    base_hud_stats_menu.y -= 0.1f;
    HUDStat firerate_stat(MHUDSTAT::kStat_FireRate);
    firerate_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStati(RebirthPlayerStat::kTearsDelay),
                       mem_reader->GetPlayerRecentStatChangei(RebirthPlayerStat::kTearsDelay));

    base_hud_stats_menu.y -= 0.1f;
    HUDStat shotspeed_stat(MHUDSTAT::kStat_ShotSpeed);
    shotspeed_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStatf(RebirthPlayerStat::kShotSpeed),
                        mem_reader->GetPlayerRecentStatChangef(RebirthPlayerStat::kShotSpeed));

    base_hud_stats_menu.y -= 0.1f;
    HUDStat damage_stat(MHUDSTAT::kStat_Damage);
    damage_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStatf(RebirthPlayerStat::kDamage),
                     mem_reader->GetPlayerRecentStatChangef(RebirthPlayerStat::kDamage));

    base_hud_stats_menu.y -= 0.1f;
    HUDStat luck_stat(MHUDSTAT::kStat_Luck);
    luck_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStatf(RebirthPlayerStat::kLuck),
                   mem_reader->GetPlayerRecentStatChangef(RebirthPlayerStat::kLuck));

    base_hud_stats_menu.y -= 0.1f;
    HUDStat dwd_stat(MHUDSTAT::kStat_DealWithDevil);
    dwd_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStatf(RebirthPlayerStat::kDealWithDevil),
                  mem_reader->GetPlayerRecentStatChangef(RebirthPlayerStat::kDealWithDevil), true);
}
