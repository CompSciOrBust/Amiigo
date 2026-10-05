#version 330 core
in vec2 texCoords;
out vec4 FragColor;

uniform vec4 colour;
uniform bool checked;

void main()
{
    float border = 0.1;
    float innerPad = 0.2;

    bool inBorder = texCoords.x < border || texCoords.x > (1.0 - border)
                 || texCoords.y < border || texCoords.y > (1.0 - border);

    bool inInnerBounds = texCoords.x > innerPad && texCoords.x < (1.0 - innerPad)
                      && texCoords.y > innerPad && texCoords.y < (1.0 - innerPad);

    vec2 uv = (texCoords - innerPad) / (1.0 - 2.0 * innerPad);
    float thickness = 0.15;
    bool onX = inInnerBounds && (abs(uv.y - uv.x) < thickness || abs(uv.y + uv.x - 1.0) < thickness);

    if (inBorder || (checked && onX)) {
        FragColor = colour;
        return;
    }

    discard;
}
