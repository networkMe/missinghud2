#version 110

varying vec2 v_texture_coord;

uniform sampler2D app_texture;
uniform vec3 texture_color;

void main()
{
    gl_FragColor = texture2D(app_texture, v_texture_coord) * vec4(texture_color, 1.0);
}