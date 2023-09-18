#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 translation;
layout (location = 2) in vec3 scale;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    mat4 innerModelMatrix = mat4(1.0F);
    innerModelMatrix[3].xyz = translation;
    innerModelMatrix[0][0] = scale.x;
    innerModelMatrix[1][1] = scale.y;
    innerModelMatrix[2][2] = scale.z;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * innerModelMatrix * vec4(position, 1.0);
}
