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

#include "SpriteSheet.h"

std::map <RESID, SpriteSheet*> SpriteSheet::spritesheets_;

SpriteSheet *SpriteSheet::LoadSpriteSheet(RESID spritesheet_res_id, RESID spritesheet_tex_map, SPRITESIZE sprite_size,
                                          bool invert_img_y)
{
    if (spritesheets_.count(spritesheet_res_id) > 0)
    {
        return spritesheets_[spritesheet_res_id];
    }

    // Create the spritesheet
    SpriteSheet* new_spritesheet = new SpriteSheet(spritesheet_res_id, spritesheet_tex_map, sprite_size, invert_img_y);

    // Add the spritesheet to our loaded list
    spritesheets_.insert(std::make_pair(spritesheet_res_id, new_spritesheet));
    return new_spritesheet;
}

SpriteSheet *SpriteSheet::GetSpriteSheet(RESID spritesheet_res_id)
{
    return spritesheets_[spritesheet_res_id];
}

void SpriteSheet::DestroySpriteSheet(RESID spritesheet_res_id)
{
    for (auto &spritesheet : spritesheets_)
    {
        if (spritesheet.first == spritesheet_res_id)
        {
            delete spritesheet.second;
            spritesheets_.erase(spritesheet.first);
            break;
        }
    }
}

void SpriteSheet::DestroyAllSpriteSheets()
{
    for (auto &spritesheet : spritesheets_)
    {
        delete spritesheet.second;
    }
    spritesheets_.clear();
}

SpriteSheet::SpriteSheet(RESID spritesheet_res_id, RESID spritesheet_tex_map, SPRITESIZE sprite_size, bool invert_img_y)
{
    LoadTexture(spritesheet_res_id, invert_img_y);
    MapSpriteTextures(spritesheet_tex_map, sprite_size);
    GenerateDynamicVertex(4);
}

SpriteSheet::~SpriteSheet()
{
    glDeleteTextures(1, &spritesheet_texture_);
    glDeleteVertexArrays(1, & vertex_array_);
    glDeleteBuffers(1, &array_buffer_);
    glDeleteBuffers(1, &ele_array_buffer_);
}

void SpriteSheet::LoadTexture(RESID spritesheet_res_id, bool invert_img_y)
{
    // Load the sprite sheet texture
    FileResource spritesheet_res = ResourceLoader::GetBinResource(spritesheet_res_id);
    int width = 0, height = 0, channels = 0;
    unsigned char* texture_img = SOIL_load_image_from_memory((unsigned char*)spritesheet_res.bin_data,
                                                             spritesheet_res.res_size, &width, &height, &channels,
                                                             SOIL_LOAD_RGBA);
    tex_width_ = width;
    tex_height_ = height;

    if (invert_img_y)
    {
        unsigned char* inverted_image = InvertImage(texture_img, width, height, channels);
        SOIL_free_image_data(texture_img);
        texture_img = inverted_image;
    }

    glGenTextures(1, &spritesheet_texture_);
    glBindTexture(GL_TEXTURE_2D, spritesheet_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width_, tex_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_img);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (invert_img_y)
        delete[] texture_img;
    else
        SOIL_free_image_data(texture_img);
}

void SpriteSheet::MapSpriteTextures(RESID spritesheet_tex_map, SPRITESIZE sprite_size)
{
    FileResource tex_map = ResourceLoader::GetBinResource(spritesheet_tex_map);
    std::string tex_map_str = std::string((const char*)tex_map.bin_data);
    std::vector<std::string> tex_map_array = ExplodeTexMap(tex_map_str);

    int num_cols = tex_width_ / sprite_size.width;
    sprite_tex_width_ = 1.0f / num_cols;
    int num_rows = tex_height_ / sprite_size.height;
    sprite_tex_height_ = 1.0f / num_rows;

    for (int tex = 0; tex < tex_map_array.size(); ++tex)
    {
        sprite_index_.insert(std::make_pair(tex_map_array[tex], tex));
    }
}

void SpriteSheet::GenerateDynamicVertex(int alloced_vertexes)
{
    // We know the element buffer before drawing (it's not relative to viewport changes)
    GLuint gl_ele_buff[] = {
        1, 0, 3,
        1, 3, 2
    };

    // Generate the dynamic vertex array for the 2D spritesheet
    glGenVertexArrays(1, &vertex_array_);
    glGenBuffers(1, &array_buffer_);
    glGenBuffers(1, &ele_array_buffer_);

    glBindVertexArray(vertex_array_);
        glBindBuffer(GL_ARRAY_BUFFER, array_buffer_);
        glBufferData(GL_ARRAY_BUFFER, alloced_vertexes * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ele_array_buffer_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gl_ele_buff), gl_ele_buff, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        // Texture coord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void SpriteSheet::DrawSprite(glm::vec2 position, glm::vec2 size, std::string sprite_name)
{
    // Get the GLSL shaders required
    ShaderProgram *spritesheet_shader = ShaderProgram::GetProgram(OPENGL_HUD_SPRITE_VERTEX_SHADER,
                                                                  OPENGL_HUD_SPRITE_FRAG_SHADER);
    if (spritesheet_shader == nullptr)
        return;

    // Get the sprites index in the spritesheet
    int sprite_index = sprite_index_[sprite_name];
    float tex_top_left_x = sprite_tex_width_ * sprite_index;
    float tex_top_left_y = sprite_tex_height_;
    GLfloat gl_vert_buff_[] = {
        // Position                                                       // Texture
        (GLfloat)position.x + size.x,    (GLfloat)position.y,             tex_top_left_x + sprite_tex_width_,    tex_top_left_y,                         // Top-right
        (GLfloat)position.x + size.x,    (GLfloat)position.y - size.y,    tex_top_left_x + sprite_tex_width_,    tex_top_left_y - sprite_tex_height_,    // Bottom-right
        (GLfloat)position.x,             (GLfloat)position.y - size.y,    tex_top_left_x,    tex_top_left_y - sprite_tex_height_,                        // Bottom-left
        (GLfloat)position.x,             (GLfloat)position.y,             tex_top_left_x,    tex_top_left_y                                              // Top-left
    };

    // Actually draw the sprite
    spritesheet_shader->Use();

    glBindTexture(GL_TEXTURE_2D, spritesheet_texture_);

    glBindBuffer(GL_ARRAY_BUFFER, array_buffer_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(gl_vert_buff_), gl_vert_buff_);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(vertex_array_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    spritesheet_shader->CloseProgram();
}

std::vector<std::string> SpriteSheet::ExplodeTexMap(const std::string &s)
{
    std::vector<std::string> exploded_tex_map;
    std::istringstream ss(s);

    std::string tex_name;
    while (std::getline(ss, tex_name, ' '))
    {
        exploded_tex_map.push_back(tex_name);
    }

    return exploded_tex_map;
}

unsigned char *SpriteSheet::InvertImage(unsigned char *soil_image_data, int width, int height, int channels)
{
    unsigned char *texture_img_copy = new unsigned char[width * height * channels]{ '\0' };
    memcpy(texture_img_copy, soil_image_data, width * height * channels);

    for (int j = 0; (j * 2) < height; ++j)
    {
        int index_1 = j * width * channels;
        int index_2 = (height - 1 - j) * width * channels;
        for (int i = width * channels; i > 0; --i)
        {
            unsigned char temp = texture_img_copy[index_1];
            texture_img_copy[index_1] = texture_img_copy[index_2];
            texture_img_copy[index_2] = temp;
            ++index_1;
            ++index_2;
        }
    }

    return texture_img_copy;
}
