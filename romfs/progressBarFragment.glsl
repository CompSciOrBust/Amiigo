#version 330 core
in vec2 texCoords;
out vec4 FragColor;

uniform vec4 colour;
uniform float progress;
uniform float aspectRatio; // barWidth / barHeight

float roundedRect(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b + r;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

void main()
{
    vec2 p = vec2(texCoords.x * aspectRatio, texCoords.y);
    float r = 0.5;
    vec2 barHalf = vec2(aspectRatio * 0.5, 0.5);

    if (roundedRect(p - barHalf, barHalf, r) > 0.0) discard;

    vec4 track = vec4(0.15, 0.15, 0.15, 1.0);
    float highlight = mix(1.0, 0.75, texCoords.y);

    if (progress > 0.0) {
        float fillHalfW = progress * aspectRatio * 0.5;
        if (roundedRect(p - vec2(fillHalfW, 0.5), vec2(fillHalfW, 0.5), r) <= 0.0) {
            FragColor = vec4(colour.rgb * highlight, colour.a);
            return;
        }
    }

    FragColor = track;
}
