#version 330 core

in vec3 position;
in vec3 in_normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

out float vHeight;
out vec3 normal;
out vec3 vPosition;

void main() {
    vHeight = position.y;
    normal = normalMatrix * in_normal;
    vec4 worldPosition = modelMatrix * vec4(position, 1.0);
    vPosition = vec3(worldPosition);
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}
