#version 330 core
in vec2 texCoords;
out vec4 color;

uniform sampler2D text;
uniform vec4 colour;

void main()
{
    color = colour * vec4(1.0, 1.0, 1.0, texture(text, texCoords).r);
}