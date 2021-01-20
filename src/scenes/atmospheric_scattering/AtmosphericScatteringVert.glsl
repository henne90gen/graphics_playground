#version 330 core

#include "NoiseLib.glsl"
#include "ScatterLib.glsl"

in vec3 a_Position;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 cameraPosition;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform float lightPower;

out vec3 normal_frag_in;
out vec3 modelPosition;
out float normalized_height;

out vec3 extinction;
out vec3 inScatter;

void main() {
    NoiseLayer noiseLayers[MAX_NUM_NOISE_LAYERS];
    for (int i = 0; i < MAX_NUM_NOISE_LAYERS; i++) {
        noiseLayers[i].enabled = false;
        noiseLayers[i].frequency = 0.0F;
        noiseLayers[i].amplitude = 0.0F;
    }
    noiseLayers[0].enabled = true;
    noiseLayers[0].frequency = 50.0F;
    noiseLayers[0].amplitude = 8.0F;

    noiseLayers[1].enabled = true;
    noiseLayers[1].frequency = 35.0F;
    noiseLayers[1].amplitude = 7.0F;

    noiseLayers[2].enabled = true;
    noiseLayers[2].frequency = 25.0F;
    noiseLayers[2].amplitude = 6.0F;

    noiseLayers[3].enabled = true;
    noiseLayers[3].frequency = 15.0F;
    noiseLayers[3].amplitude = 4.0F;

    noiseLayers[4].enabled = true;
    noiseLayers[4].frequency = 8.0F;
    noiseLayers[4].amplitude = 2.0F;

    int numNoiseLayers = 5;
    float power = 1.1F;
    vec2 pos2D = a_Position.xz;
    bool useFiniteDifferences = false;
    float platformHeight = 0.0F;
    int seed = 1337;
    vec4 noise = generateHeight(pos2D, noiseLayers, numNoiseLayers, useFiniteDifferences, platformHeight, power, seed);
    float height = noise.x;
    vec4 position = modelMatrix * vec4(a_Position.x, height, a_Position.z, 1.0F);
    modelPosition = position.xyz;
    gl_Position = projectionMatrix * viewMatrix * position;

    normalized_height = noise.w;
    vec3 tangent = vec3(1.0F, noise.y, 0.0F);
    vec3 bitangent = vec3(0.0F, noise.z, 1.0F);
    normal_frag_in = -normalize(cross(tangent, bitangent));

    calcScattering(cameraPosition, modelPosition, lightDirection, lightColor, lightPower, extinction, inScatter);
}
