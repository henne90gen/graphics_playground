#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_UV;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

void main() {
    vec4 viewPos = viewMatrix * modelMatrix * vec4(a_Position, 1.0);
    FragPos = viewPos.xyz;
    TexCoords = a_UV;

    mat3 normalMatrix = transpose(inverse(mat3(viewMatrix * modelMatrix)));
    Normal = normalMatrix * a_Normal;

    gl_Position = projectionMatrix * viewPos;
}
