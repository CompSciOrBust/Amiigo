#version 330 core
in vec2 texCoords;
out vec4 FragColor;

uniform sampler2D _texture;
uniform vec4 colour;
uniform float iTime;

void main()
{
    vec4 col = vec4(0.0);
    vec2 uv = texCoords - 0.5;
    uv *= 200;
    col.a = 1.0;
    col.b = 1.0/distance(floor(uv.y / 4.0), 100.0 * sin(iTime * 0.2 + floor(uv.x))) * 4.0;
    col.b += 1.0/distance(floor(uv.x / 2.0), 100.0 * cos(iTime * 0.2 + floor(uv.y))) * 4.0;
    col.r = col.b;
    col.g = col.b;

    FragColor = col;
}