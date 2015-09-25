#version 330 core

in vec2 v_texture_coord;

out vec4 color;

uniform sampler2D app_texture;
uniform vec3 texture_color;

void main()
{
    color = texture(app_texture, v_texture_coord) * vec4(texture_color, 1.0f);
}