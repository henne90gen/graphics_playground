#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 translation;
layout (location = 2) in vec3 scale;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    mat4 modelMatrix = mat4(1.0F);
    modelMatrix[3].xyz = translation;
    modelMatrix[0][0] = scale.x;
    modelMatrix[1][1] = scale.y;
    modelMatrix[2][2] = scale.z;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}
