#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
layout (location = 1) in float diffuse;

out vec2 tex_coords;
out float text_diffuse;

uniform mat4 projection;

void main()
{
    tex_coords = vertex.zw;
    text_diffuse = diffuse;

    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
}
