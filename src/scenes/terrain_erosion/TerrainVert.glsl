#version 300 es
precision mediump float;

in vec2 position;
in float height;
in vec3 in_normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

out float vHeight;
out vec3 vNormal;
out vec3 vPosition;

void main() {
    vHeight = height;
    vNormal = normalMatrix * in_normal;
    vec3 finalPosition = vec3(position.x, height, position.y);
    vec4 worldPosition = modelMatrix * vec4(finalPosition, 1.0F);
    vPosition = vec3(worldPosition);
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}
