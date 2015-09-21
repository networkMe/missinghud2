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

#ifndef MISSINGHUD2_GLSTRUCTS_H
#define MISSINGHUD2_GLSTRUCTS_H

#include <tuple>

#include <GL/glew.h>
#include <glm/glm.hpp>

struct STDSIZE
{
    STDSIZE() {};
    STDSIZE(uint32_t _width, uint32_t _height)
    {
        width = _width;
        height = _height;
    };

    uint32_t width = 0;
    uint32_t height = 0;
};
bool operator !=(const STDSIZE & x, const STDSIZE & y);
typedef STDSIZE VIEWSIZE;
typedef STDSIZE SPRITESIZE;

struct BitmapChar
{
    BitmapChar() {};
    BitmapChar(GLuint _texture_id, glm::ivec2 _size, glm::ivec2 _baseline_offset, GLuint _advance_width)
    {
        texture_id = _texture_id;
        size = _size;
        baseline_offset = _baseline_offset;
        advance_width = _advance_width;
    }

    GLuint texture_id = 0;
    glm::ivec2 size;
    glm::ivec2 baseline_offset;
    GLuint advance_width = 0;
};

struct Color
{
    Color() {};
    Color(uint32_t _r, uint32_t _g, uint32_t _b)
    {
        r = _r;
        g = _g;
        b = _b;
    };

    uint32_t r;
    uint32_t g;
    uint32_t b;
};

#endif //MISSINGHUD2_GLSTRUCTS_H
