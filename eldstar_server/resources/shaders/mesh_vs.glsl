#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColor;

out vec3 fPos;
out vec3 fNormal;
out vec4 fColor;

uniform mat4 camera;
uniform mat4 model;
uniform vec4 globalColor;
uniform int colorMode;


void main()
{
    vec3 normal_head = vec3(model * vec4(aNormal, 1.0));
    vec3 normal_tail = vec3(model * vec4(0.0, 0.0, 0.0, 1.0));
    vec3 normal = normal_head - normal_tail;
    vec4 pos = model * vec4(aPos, 1.0);

    fPos = vec3(pos);
    fNormal = normal;

    vec4 outColor = aColor * globalColor;

    if (colorMode == 0)
        fColor = outColor;
    else if (colorMode == 1 || colorMode == 2) {
        if (normal.y > 0.01)
            fColor = vec4(0.25, 0.25, 1.0, outColor.w);
        else if (normal.y < -0.01)
            fColor = vec4(1.0, 0.25, 0.25, outColor.w);
        else
            fColor = vec4(0.25, 1.0, 0.25, outColor.w);
    }

    gl_Position = camera * pos;
}
