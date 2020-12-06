#version 330 core

in vec2 position;
in float height;
in vec3 normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

out float vHeight;
out vec3 vNormal;

void main() {
    vHeight = height;
    vNormal = normalMatrix * normal;
    vec3 finalPosition = vec3(position.x, height, position.y);
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(finalPosition, 1.0);
}
