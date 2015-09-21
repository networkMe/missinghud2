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

#include "ShaderProgram.h"

std::map<PROGFILES, ShaderProgram*> ShaderProgram::instances_;

ShaderProgram::ShaderProgram()
{

}

ShaderProgram *ShaderProgram::GetProgram(int vertex_res, int fragment_res)
{
    // Check whether we already have the program compiled
    PROGFILES prog_files = vertex_res + fragment_res;
    if (instances_.count(prog_files) > 0)
        return instances_[prog_files];

    // Nope, we don't have it cached, gotta compile and link the program
    ShaderProgram *shader_program = new ShaderProgram();

    if (!shader_program->CompileProgram(vertex_res, fragment_res))
        return nullptr;

    // Save the PROGFILES for this ShaderProgram
    instances_.insert(std::make_pair(prog_files, shader_program));

    return shader_program;
}

void ShaderProgram::DestroyAll()
{
    for (auto &shader_program : instances_)
    {
        delete shader_program.second;
    }
    instances_.clear();
}

bool ShaderProgram::CompileProgram(int vertex_res, int fragment_res)
{
    // Compile the vertex shader
    FileResource vertex_fileres = ResourceLoader::GetBinResource(vertex_res);
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (char**)&vertex_fileres.bin_data, NULL);
    glCompileShader(vertex_shader);

    // Make sure vertex shader compiled successfully
    GLint compile_result;
    GLchar info_log[512] = { '\0' };
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_result);
    if (!compile_result)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
        OutputDebugString(info_log);
        return false;
    }

    // Compile the fragment shader
    FileResource fragment_fileres = ResourceLoader::GetBinResource(fragment_res);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (char**)&fragment_fileres.bin_data, NULL);
    glCompileShader(fragment_shader);

    // Make sure fragment shader compiled successfully
    compile_result = 0;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_result);
    if (!compile_result)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
        OutputDebugString(info_log);
        return false;
    }

    // Link our shaders
    compiled_prog_id_ = glCreateProgram();
    glAttachShader(compiled_prog_id_, vertex_shader);
    glAttachShader(compiled_prog_id_, fragment_shader);
    glLinkProgram(compiled_prog_id_);

    // Make sure link was successful
    GLint link_result;
    glGetProgramiv(compiled_prog_id_, GL_LINK_STATUS, &link_result);
    if (!link_result)
    {
        glGetProgramInfoLog(compiled_prog_id_, 512, NULL, info_log);
        OutputDebugString(info_log);
        return false;
    }

    // Clean-up our shaders
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return true;
}
