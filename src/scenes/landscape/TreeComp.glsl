#version 430 core

#include "NoiseLib.glsl"

layout (local_size_x = 1, local_size_y = 1) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;

uniform int gridWidth = 1000;

uniform NoiseLayer noiseLayers[MAX_NUM_NOISE_LAYERS];
uniform int numNoiseLayers;
uniform float finiteDifference;
uniform bool useFiniteDifferences;
uniform float power;
uniform float bowlStrength;
uniform float platformHeight;
uniform int seed;

//void placementRandom(inout vec2 pos) {
//    pos = vec2(gold_noise(pos.xx, float(gl_InstanceID)), gold_noise(pos.yy, float(gl_InstanceID)));
//}

vec2 rotate(vec2 v, float a) {
    float s = sin(a);
    float c = cos(a);
    mat2 m = mat2(c, -s, s, c);
    return m * v;
}

void placementJiggleAroundPos(inout vec2 pos) {
    vec2 displacement = vec2(1.0F, 0.0);

    float angle = snoise2(pos*100.0F).x;
    displacement = rotate(displacement, angle);

    float distance = snoise2(pos*100.0F).x / 10.0F;
    displacement *= distance;

    pos += displacement;
}

bool discardTree(vec2 pos, vec4 noise) {
    float platformSize = 1000.0F / 15.0F;
    if ((pos.x > -platformSize && pos.x < platformSize) && (pos.y > -platformSize && pos.y < platformSize)) {
        return true;
    }

    vec3 tangent = vec3(1.0F, noise.y, 0.0F);
    vec3 bitangent = vec3(0.0F, noise.z, 1.0F);
    vec3 normal = -normalize(cross(tangent, bitangent));
    float maxSlope = 0.7F;
    if (normal.y > maxSlope) {
        return true;
    }

    return false;
}

void main() {
    // 0.0F - imageSize(imgOutput)
    vec2 pos = vec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
    // 0.0F - 1.0F
    pos /= vec2(imageSize(imgOutput).x, imageSize(imgOutput).y);
    // 0.0F - 1000.0F
    pos *= gridWidth;
    // -500.0F - 500.0F
    pos -= gridWidth / 2.0F;

    vec4 noise = generateHeight(pos, noiseLayers, numNoiseLayers, useFiniteDifferences, finiteDifference, power, bowlStrength, platformHeight, seed);
    vec3 position = vec3(pos.x, noise.x, pos.y);

    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    imageStore(imgOutput, pixelCoords, vec4(position, 1.0F));
}
