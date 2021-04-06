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
    #if 1
    ivec2 positionTextureSize = textureSize(positionTexture, 0);
    vec2 uv = vec2(0.0F, 0.0F);
    uv.x = float(gl_InstanceID % positionTextureSize.x) / float(positionTextureSize.x);
    uv.y = float(gl_InstanceID / positionTextureSize.y) / float(positionTextureSize.y);
    vec3 positionOffset = texture(positionTexture, uv).xyz;
    #else
    vec3 positionOffset = vec3(0.0);
    #endif

    vec4 viewPosition = viewMatrix * (vec4(positionOffset, 0.0F) + modelMatrix * vec4(a_Position, 1.0F));
    position_frag_in = viewPosition.xyz;
    gl_Position = projectionMatrix * viewPosition;
    normal_frag_in = normalMatrix * a_Normal;
    uv_frag_in = a_UV;
}
