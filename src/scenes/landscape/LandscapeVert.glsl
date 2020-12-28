#version 330 core

#if 1

layout (location = 0) in vec2 position_in;

out vec2 position_tcs_in;

uniform mat4 viewMatrix;

void main() {
    position_tcs_in = position_in;
}

#else

in vec2 position;
in vec2 uv;
in vec3 normal;
in vec3 tangent;
in vec3 biTangent;

uniform mat3 normalMatrix;
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

    vec2 seed = vec2(0.0F, 0.0F);
    vHeight = snoise2(position+seed);

    vNormal = normalMatrix * normal;
    vec3 finalPosition = vec3(position.x, vHeight, position.y);
    vec4 worldPosition = modelMatrix * vec4(finalPosition, 1.0);
    vPosition = vec3(worldPosition);
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}

#endif
