#version 110

uniform mat4 projection;

attribute vec2 position;
attribute vec2 texture_coord;

varying vec2 v_texture_coord;

void main()
{
    v_texture_coord = texture_coord;
    gl_Position = projection * vec4(position.xy, 0.0, 1.0);
}