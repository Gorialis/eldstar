#version 330 core

in vec2 tex_coords;
in float text_diffuse;

out vec4 color;

uniform sampler2D text;
uniform vec3 text_color;

void main()
{
    vec4 sampled = vec4(text_diffuse, text_diffuse, text_diffuse, texture(text, tex_coords).r);
    color = vec4(text_color, 1.0) * sampled;
}
