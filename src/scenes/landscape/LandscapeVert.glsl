#version 330 core

in vec2 position;
in float height;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out float vHeight;

void main() {
    vHeight = height;
    vec3 finalPosition = vec3(position.x, height, position.y);
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(finalPosition, 1.0);
}
