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

#ifndef MISSINGHUD2_SPRITESHEET_H
#define MISSINGHUD2_SPRITESHEET_H

#include <map>
#include <sstream>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL2.h>

#include "GLStructs.h"
#include "ResourceLoader.h"
#include "ShaderProgram.h"
#include "HUDOverlay.h"
#include <res/DllResources.h>

class SpriteSheet
{
public:
    static SpriteSheet *LoadSpriteSheet(RESID spritesheet_res_id, RESID spritesheet_tex_map, SPRITESIZE sprite_size,
                                        bool invert_img_y);
    static SpriteSheet *GetSpriteSheet(RESID spritesheet_res_id);
    static void DestroySpriteSheet(RESID spritesheet_res_id);
    static void DestroyAllSpriteSheets();

    void DrawSprite(glm::vec2 position, glm::vec2 size, std::string sprite_name,
                    glm::vec3 sprite_color = glm::vec3(1.0f, 1.0f, 1.0f));

private:
    SpriteSheet(RESID spritesheet_res_id, RESID spritesheet_tex_map, SPRITESIZE sprite_size, bool invert_img_y);
    ~SpriteSheet();

    void LoadTexture(RESID spritesheet_res_id, bool invert_img_y);
    void MapSpriteTextures(RESID spritesheet_tex_map, SPRITESIZE sprite_size);

    unsigned char *InvertImage(unsigned char *soil_image_data, int width, int height, int channels);
    std::vector<std::string> ExplodeTexMap(const std::string &s);

private:
    static std::map<RESID, SpriteSheet*> spritesheets_;

    GLuint spritesheet_texture_ = 0;
    GLuint array_buffer_ = 0, ele_array_buffer_ = 0;
    int tex_height_ = 0;
    int tex_width_ = 0;

    std::map<std::string, int> sprite_index_;
    float sprite_tex_width_ = 0.0;
    float sprite_tex_height_ = 0.0;
};


#endif //MISSINGHUD2_SPRITESHEET_H
