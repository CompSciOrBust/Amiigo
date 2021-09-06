#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 texureCordsIn;
out vec2 texCoords;

uniform mat4 transform;
uniform mat4 projection;

void main()
{
    gl_Position = projection * transform * vec4(vertex.x, vertex.y, vertex.z, 1.0);
    texCoords = texureCordsIn;
}