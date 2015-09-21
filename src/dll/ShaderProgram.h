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

#ifndef MISSINGHUD2_SHADERPROGRAM_H
#define MISSINGHUD2_SHADERPROGRAM_H

#include <map>

#include <windows.h>
#include <GL/glew.h>

#include "ResourceLoader.h"

typedef int PROGFILES;

class ShaderProgram
{
public:
    static ShaderProgram* GetProgram(int vertex_res, int fragment_res);
    static void DestroyAll();

    void Use()
    {
        glUseProgram(compiled_prog_id_);
    };

    void CloseProgram()
    {
        glUseProgram(0);
    };

    GLuint GetProgram()
    {
        return compiled_prog_id_;
    };

private:
    ShaderProgram();

    bool CompileProgram(int vertex_res, int fragment_res);

private:
    static std::map<PROGFILES, ShaderProgram*> instances_;

    GLuint compiled_prog_id_ = 0;
};

#endif //MISSINGHUD2_SHADERPROGRAM_H
