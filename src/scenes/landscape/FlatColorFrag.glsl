#version 330 core

in vec3 model_position;
in vec3 normal_frag_in;

uniform vec3 flatColor;
uniform mat3 normalMatrix;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedoSpec;
layout (location = 3) out vec3 gDoLighting;

void main() {
    gPosition = model_position;
    gNormal = normalize(normalMatrix * normal_frag_in);
    gAlbedoSpec = flatColor;
    gDoLighting = vec3(0.0F);
}
