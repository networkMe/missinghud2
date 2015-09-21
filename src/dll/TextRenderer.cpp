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

#include "TextRenderer.h"

std::map<RESID, TextRenderer*> TextRenderer::renderers_;

TextRenderer * TextRenderer::GetRenderer(RESID font_res_id, RESID font_charmap)
{
    if (renderers_.count(font_res_id) > 0)
    {
        return renderers_[font_res_id];
    }

    // Create the renderer
    TextRenderer *text_renderer = new TextRenderer(font_res_id, font_charmap);

    // Add it to our map list
    renderers_.insert(std::make_pair(font_res_id, text_renderer));
    return text_renderer;
}

void TextRenderer::DestroyAll()
{
    for (auto &renderer : renderers_)
    {
        delete renderer.second;
    }

    renderers_.clear();
}

TextRenderer::TextRenderer(RESID font_res_id, RESID font_charmap)
{
    res_id_ = font_res_id;
    SpriteSheet::LoadSpriteSheet(font_res_id, font_charmap, SPRITESIZE(16, 16), true);
}

TextRenderer::~TextRenderer()
{
    SpriteSheet::DestroySpriteSheet(res_id_);
}

void TextRenderer::RenderText(glm::vec2 position, std::string text)
{
    SpriteSheet *text_sprites = SpriteSheet::GetSpriteSheet(res_id_);

    for (char text_ch : text)
    {
        char sprite_letter[2] = { text_ch, '\0' };
        text_sprites->DrawSprite(position, glm::vec2(0.04f, 0.06f), std::string(sprite_letter));
        position.x += 0.020f;
    }
}
