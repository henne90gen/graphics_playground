#version 330 core

#include "NoiseLib.glsl"

in vec3 a_Position;
in vec3 a_Normal;
in vec2 a_UV;

out vec3 position_frag_in;
out vec3 normal_frag_in;
out vec2 uv_frag_in;

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform sampler2D positionTexture;

void main() {
    #if 0
    ivec2 positionTextureSize = textureSize(positionTexture, 0);
    vec2 uv = vec2(0.0F, 0.0F);
    uv.x = float(gl_InstanceID % positionTextureSize.x) / float(positionTextureSize.x);
    uv.y = float(gl_InstanceID / positionTextureSize.y) / float(positionTextureSize.y);
    vec3 position = a_Position + texture(positionTexture, uv).xyz;
    #else
    vec3 position = a_Position;
    #endif

    vec4 viewPosition = viewMatrix * modelMatrix * vec4(position, 1.0F);
    position_frag_in = viewPosition.xyz;
    gl_Position = projectionMatrix * viewPosition;
    normal_frag_in = normalMatrix * a_Normal;
    uv_frag_in = a_UV;
}
