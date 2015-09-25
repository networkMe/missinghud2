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

#ifndef MISSINGHUD2_TEXTRENDERER_H
#define MISSINGHUD2_TEXTRENDERER_H

#include <GL/glew.h>
#include <SOIL2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLStructs.h"
#include "ShaderProgram.h"
#include "SpriteSheet.h"
#include "ResourceLoader.h"
#include <res/DllResources.h>

class TextRenderer
{
public:
    static TextRenderer *GetRenderer(RESID font_res_id, RESID font_charmap);
    static void DestroyAll();

    // Returns a vector object specifying the OpenGL height/width of the rendered text
    glm::vec2 RenderText(glm::vec2 position, std::string text, Color text_color);

private:
    TextRenderer(RESID font_res_id, RESID font_charmap);
    ~TextRenderer();

private:
    static std::map<RESID, TextRenderer*> renderers_;
    RESID res_id_ = 0;
};


#endif //MISSINGHUD2_TEXTRENDERER_H
