#version 330 core

in vec2 position;
in vec2 uv;
in float height;
in vec3 normal;
in vec3 tangent;
in vec3 biTangent;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform float uvScaleFactor;

out float vHeight;
out vec3 vPosition;
out vec2 vUV;
out vec3 vNormal;
out mat3 vTBN;
out vec3 vCameraPosition;

void main() {
    vec3 T = normalize(vec3(modelMatrix * vec4(tangent, 0.0)));
    vec3 B = normalize(vec3(modelMatrix * vec4(biTangent, 0.0)));
    vec3 N = normalize(vec3(modelMatrix * vec4(normal, 0.0)));
    vTBN = mat3(T, B, N);
    vCameraPosition = vec3(viewMatrix * vec4(0.0, 0.0, 0.0, 1.0));
    vUV = uv/uvScaleFactor;
    vHeight = height;
    vNormal = normal;
    vec3 finalPosition = vec3(position.x, height, position.y);
    vec4 worldPosition = modelMatrix * vec4(finalPosition, 1.0);
    vPosition = vec3(worldPosition);
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}
