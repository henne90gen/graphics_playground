#version 440 core

#include "NoiseLib.glsl"
#include "ScatterLib.glsl"

layout (triangles, equal_spacing, cw) in;

in vec2 position_tes_in[];
in vec3 ev_tes_in[];

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform float uvScaleFactor;
uniform vec3 cameraPosition;

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 atmosphere;

uniform NoiseLayer noiseLayers[MAX_NUM_NOISE_LAYERS];
uniform int numNoiseLayers;
uniform bool useFiniteDifferences;
uniform float finiteDifference;
uniform float power;
uniform float bowlStrength;
uniform float platformHeight;
uniform int seed;

out vec2 uv_frag_in;
out vec3 normal_frag_in;
out vec3 tangent_frag_in;
out vec3 bitangent_frag_in;
out vec3 model_position;
out float normalized_height;

out vec3 extinction;
out vec3 inScatter;

void main() {
    vec2 pos = gl_TessCoord.x * position_tes_in[0];
    pos     += gl_TessCoord.y * position_tes_in[1];
    pos     += gl_TessCoord.z * position_tes_in[2];

    vec4 noise = generateHeight(pos, noiseLayers, numNoiseLayers, useFiniteDifferences, finiteDifference, power, bowlStrength, platformHeight, seed);
    normalized_height = noise.w;
    tangent_frag_in = vec3(1.0F, noise.y, 0.0F);
    bitangent_frag_in = vec3(0.0F, noise.z, 1.0F);
    normal_frag_in = -normalize(cross(tangent_frag_in, bitangent_frag_in));

    vec4 position = modelMatrix * vec4(pos.x, noise.x, pos.y, 1.0F);
    model_position = position.xyz;
    gl_Position = projectionMatrix * viewMatrix * position;

    uv_frag_in = pos / uvScaleFactor;

    float lightPower = 200;
    calcScattering(cameraPosition, model_position, -lightDirection, lightColor, lightPower, atmosphere, extinction, inScatter);
}
