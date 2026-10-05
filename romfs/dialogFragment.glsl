#version 330 core
in vec2 texCoords;
out vec4 FragColor;

uniform vec4 colour;
uniform float aspectRatio;
uniform float radius;
uniform float outlineWidth;

float roundedRect(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b + r;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

void main()
{
    vec2 p = vec2(texCoords.x * aspectRatio, texCoords.y);
    vec2 rectHalf = vec2(aspectRatio * 0.5, 0.5);

    float dist = roundedRect(p - rectHalf, rectHalf, radius);

    if (dist > 0.0) discard;
    if (dist > -outlineWidth) { FragColor = vec4(0.0, 0.0, 0.0, 1.0); return; }

    FragColor = colour;
}
