#version 330 core

in vec2 position;
in float height;
in vec3 in_normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out float vHeight;
out vec3 normal;

void main() {
    vHeight = height;
    normal = in_normal;
    vec3 finalPosition = vec3(position.x, height, position.y);
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(finalPosition, 1.0);
}
