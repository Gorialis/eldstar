#version 330 core

in vec3 fPos;
in vec3 fNormal;
in vec4 fColor;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 camPos;
uniform int colorMode;


void main()
{
    // ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);

    // diffuse
    vec3 norm = normalize(fNormal);
    vec3 diffuse = vec3(1.0, 1.0, 1.0);

    if(dot(norm, normalize(fPos - camPos)) >= 0) {
        if (colorMode == 2) {
            discard;
        } else {
            norm = -norm;
        }
    }

    vec3 lightDir = normalize(lightPos - fPos);
    float diff = max(dot(norm, lightDir), 0.0);
    diffuse *= diff;

    float distance = max(min((length(fPos - camPos) - 40) / 150.0, 1.0), 0.0);

    vec3 result = (ambient + diffuse) * fColor.xyz;
    FragColor = vec4(result, fColor.w * distance);
}
