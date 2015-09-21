#version 330 core

in vec2 v_texture_coord;

out vec4 color;

uniform sampler2D app_texture;

void main()
{
    color = texture(app_texture, v_texture_coord);
}