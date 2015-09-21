#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texture_coord;

out vec2 v_texture_coord;

void main()
{
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
    v_texture_coord = texture_coord;
}