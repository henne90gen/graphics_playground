#version 330 core

in vec3 position_frag_in;
in vec3 normal_frag_in;
in vec2 uv_frag_in;

uniform sampler2D textureSampler;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gExtinction;
layout (location = 4) out vec4 gInScatter;
layout (location = 5) out vec4 gDoLighting;

void main() {
    vec4 color = texture(textureSampler, uv_frag_in);

    // make trees brighter
    color.xyz *= 3.0F;

    gPosition = vec4(position_frag_in, 1.0F);
    gNormal = vec4(normalize(normal_frag_in), 1.0F);
    gAlbedoSpec = color;
    gDoLighting = vec4(1.0F);
}
