#version 330 core

in vec3 position;
in vec3 in_normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

out float vHeight;
out float batchId;
out vec3 vPosition;
out vec3 normal;

void main() {
    vHeight = position.y;
    batchId = in_normal.y;
    normal = normalMatrix * vec3(in_normal.x, 1.0F, in_normal.z);
    vec4 worldPosition = modelMatrix * vec4(position, 1.0);
    vPosition = vec3(worldPosition);
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}
