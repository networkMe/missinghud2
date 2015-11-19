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

VIEWSIZE HUDOverlay::GetViewportSize()
{
    // Check viewport size every 2 seconds (to avoid spamming glGetIntegerv)
    if ((viewport_size_.width != 0 || viewport_size_.height != 0) &&
        (viewport_updated_ > std::chrono::system_clock::now() - std::chrono::milliseconds(2000)))
        return viewport_size_;

    GLint viewport_vals[4] = { 0 };
    glGetIntegerv(GL_VIEWPORT, viewport_vals);
    int viewport_width = viewport_vals[2];
    int viewport_height = viewport_vals[3];

    viewport_size_ = VIEWSIZE(viewport_width, viewport_height);
    viewport_updated_ = std::chrono::system_clock::now();

    return viewport_size_;
}

float HUDOverlay::GetHUDSizeMultiplier()
{
    VIEWSIZE vp_size = HUDOverlay::GetViewportSize();
    if (vp_size.height < 720 || vp_size.width < 1200)
        return 1.0f;
    else if (vp_size.height < 960 || vp_size.width < 1600)
        return 1.5f;
    else
        return 2.0f;
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
    MemReader *mem_reader = MemReader::GetMemoryReader();
    if (!mem_reader->IsRunActive())
        return; // We don't want to draw the HUD if the player isn't in a Rebirth run

    // Draw the HUD stats info bar
    glm::vec2 base_hud_stats_menu;
    base_hud_stats_menu.x = 0.0f;

    VIEWSIZE vp_size = GetViewportSize();
    base_hud_stats_menu.y = (vp_size.height / 24) * 15;

    // Show the actual HUD stats
    if (DLLPreferences::GetInstance()->GetPrefs().show_tears_fired)
    {
        HUDStat tears_fired_stat(MHUDSTAT::kStat_TearsFired);
        tears_fired_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStati(PlayerStat::kTearsFired),
                              NO_RECENT_STAT_CHANGES_I);

        base_hud_stats_menu.y -= (25.0f * GetHUDSizeMultiplier());
    }

    HUDStat speed_stat(MHUDSTAT::kStat_Speed);
    speed_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStatf(PlayerStat::kSpeed),
                    mem_reader->GetPlayerRecentStatChangef(PlayerStat::kSpeed));

    base_hud_stats_menu.y -= (25.0f * GetHUDSizeMultiplier());
    HUDStat range_stat(MHUDSTAT::kStat_Range);
    range_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStatf(PlayerStat::kRange),
                    mem_reader->GetPlayerRecentStatChangef(PlayerStat::kRange));

    base_hud_stats_menu.y -= (25.0f * GetHUDSizeMultiplier());
    HUDStat firerate_stat(MHUDSTAT::kStat_FireRate);
    firerate_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStati(PlayerStat::kTearsDelay),
                       mem_reader->GetPlayerRecentStatChangei(PlayerStat::kTearsDelay));

    base_hud_stats_menu.y -= (25.0f * GetHUDSizeMultiplier());
    HUDStat shotspeed_stat(MHUDSTAT::kStat_ShotSpeed);
    shotspeed_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStatf(PlayerStat::kShotSpeed),
                        mem_reader->GetPlayerRecentStatChangef(PlayerStat::kShotSpeed));

    if (DLLPreferences::GetInstance()->GetPrefs().show_shot_height)
    {
        base_hud_stats_menu.y -= (25.0f * GetHUDSizeMultiplier());
        HUDStat shot_height_stat(MHUDSTAT::kStat_ShotHeight);
        shot_height_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStatf(PlayerStat::kShotHeight),
                              mem_reader->GetPlayerRecentStatChangef(PlayerStat::kShotHeight));
    }

    base_hud_stats_menu.y -= (25.0f * GetHUDSizeMultiplier());
    HUDStat damage_stat(MHUDSTAT::kStat_Damage);
    damage_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStatf(PlayerStat::kDamage),
                     mem_reader->GetPlayerRecentStatChangef(PlayerStat::kDamage));

    base_hud_stats_menu.y -= (25.0f * GetHUDSizeMultiplier());
    HUDStat luck_stat(MHUDSTAT::kStat_Luck);
    luck_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStatf(PlayerStat::kLuck),
                   mem_reader->GetPlayerRecentStatChangef(PlayerStat::kLuck));

    if (DLLPreferences::GetInstance()->GetPrefs().split_deal_chance)
    {
        base_hud_stats_menu.y -= (25.0f * GetHUDSizeMultiplier());
        HUDStat dwd_stat(MHUDSTAT::kStat_DealWithDevil);
        dwd_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStatf(PlayerStat::kDealWithDevil),
                      mem_reader->GetPlayerRecentStatChangef(PlayerStat::kDealWithDevil), SHOW_AS_PERCENTAGE);

        base_hud_stats_menu.y -= (25.0f * GetHUDSizeMultiplier());
        HUDStat dwa_stat(MHUDSTAT::kStat_DealWithAngel);
        dwa_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStatf(PlayerStat::kDealWithAngel),
                      mem_reader->GetPlayerRecentStatChangef(PlayerStat::kDealWithAngel), SHOW_AS_PERCENTAGE);
    }
    else
    {
        if (!mem_reader->PlayingGreed())    // Non-split deal chance is irrelevant on Greed mode
        {
            base_hud_stats_menu.y -= (25.0f * GetHUDSizeMultiplier());
            HUDStat dwd_stat(MHUDSTAT::kStat_DealDoorChance);
            dwd_stat.Draw(base_hud_stats_menu, mem_reader->GetPlayerStatf(PlayerStat::kDealDoorChance),
                          mem_reader->GetPlayerRecentStatChangef(PlayerStat::kDealDoorChance), SHOW_AS_PERCENTAGE);
        }
    }
}
