#version 330 core

in vec3 model_position;
in vec3 normal_frag_in;

uniform vec3 flatColor;
uniform mat3 normalMatrix;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gDoLighting;

void main() {
    gPosition = vec4(model_position, 1.0F);
    gNormal = vec4(normalize(normalMatrix * normal_frag_in), 1.0F);
    gAlbedoSpec = vec4(flatColor, 1.0F);
    gDoLighting = vec4(1.0F);
}
