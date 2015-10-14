#version 110

attribute vec2 position;
attribute vec2 texture_coord;

varying vec2 v_texture_coord;

void main()
{
    v_texture_coord = texture_coord;
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
}