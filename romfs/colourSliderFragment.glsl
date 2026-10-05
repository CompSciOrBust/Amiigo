#version 330 core
in vec2 texCoords;
out vec4 FragColor;

uniform vec4 colour;
uniform float progress;

void main()
{
    if (texCoords.x <= progress)
        FragColor = mix(vec4(0.0, 0.0, 0.0, 1.0), colour, texCoords.x);
    else
        FragColor = vec4(0.2, 0.2, 0.2, 1.0);
}
