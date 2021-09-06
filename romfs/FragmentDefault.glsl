#version 330 core
in vec2 texCoords;
out vec4 FragColor;

uniform sampler2D _texture;
uniform vec4 colour;

void main()
{
    FragColor = colour * texture(_texture, texCoords);
}