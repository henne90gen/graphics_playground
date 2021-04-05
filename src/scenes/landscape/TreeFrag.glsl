#version 330 core

in vec3 position_frag_in;
in vec3 normal_frag_in;
in vec2 uv_frag_in;

uniform mat3 normalMatrix;
uniform vec3 flatColor;
uniform sampler2D textureSampler;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedoSpec;
layout (location = 5) out vec3 gDoLighting;

void main() {
    vec3 color = texture(textureSampler, uv_frag_in).rgb;

    gPosition = position_frag_in;
    gNormal = normalize(normalMatrix * normal_frag_in);
    gAlbedoSpec = color*1;
}
