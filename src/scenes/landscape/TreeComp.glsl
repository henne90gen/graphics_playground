#version 430 core

#include "NoiseLib.glsl"

layout (local_size_x = 1, local_size_y = 1) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;

uniform float lod0Size;
uniform float lod0InnerSize = 100.0F;
uniform float lod1Size;
uniform float lod2Size;
uniform int gridWidth = 1000;
uniform int treeCount;

uniform NoiseLayer noiseLayers[MAX_NUM_NOISE_LAYERS];
uniform int numNoiseLayers;
uniform float finiteDifference;
uniform bool useFiniteDifferences;
uniform float power;
uniform float bowlStrength;
uniform float platformHeight;
uniform int seed;

void placementRandom(inout vec2 pos, int seed) {
    pos = vec2(seed, seed);
    pos = vec2(snoise2(pos*pos).x, snoise2(pos).x);
    pos += 1.0F;
    pos /= 2.0F;
}

vec2 rotate(vec2 v, float a) {
    float s = sin(a);
    float c = cos(a);
    mat2 m = mat2(c, -s, s, c);
    return m * v;
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
    ivec2 imgSize = imageSize(imgOutput);
    int invocationId = int(gl_GlobalInvocationID.x + imgSize.y * gl_GlobalInvocationID.y);
    if (invocationId > treeCount) {
        // don't calculate positions that won't be used
        ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
        imageStore(imgOutput, pixelCoords, vec4(0.0F, 0.0F, 0.0F, 1.0F));
        return;
    }

    // 0.0F - imageSize(imgOutput)
    vec2 pos = vec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
    // 0.0F - 1.0F
    pos /= vec2(imgSize.x, imgSize.y);

    placementRandom(pos, invocationId);

    if (invocationId < treeCount / 2) {
        // LOD 0
        int lodId = int((float(invocationId) / (float(treeCount) / 2.0F)) * 8.0F);

        const float lod0H = lod0Size / 2.0F;
        const float lod0IH = lod0InnerSize / 2.0F;
        const float smallSideLength = (lod0Size - lod0InnerSize) / 2.0F;
        const vec2 scales[8] = vec2[8](
        vec2(smallSideLength, smallSideLength),
        vec2(lod0InnerSize, smallSideLength),
        vec2(smallSideLength, smallSideLength),

        vec2(smallSideLength, lod0InnerSize),
        vec2(smallSideLength, lod0InnerSize),

        vec2(smallSideLength, smallSideLength),
        vec2(lod0InnerSize, smallSideLength),
        vec2(smallSideLength, smallSideLength)
        );

        const vec2 offsets[8] = vec2[8](
        vec2(-lod0H, lod0IH),
        vec2(-lod0IH, lod0IH),
        vec2(lod0IH, lod0IH),

        vec2(-lod0H, -lod0IH),
        vec2(lod0IH, -lod0IH),

        vec2(-lod0H, -lod0H),
        vec2(-lod0IH, -lod0H),
        vec2(lod0IH, -lod0H)
        );

        pos *= scales[lodId];
        pos += offsets[lodId];
    } else if (invocationId < treeCount / 4 * 3) {
        // LOD 1
        int lodId = int(float(invocationId-treeCount/2) / (float(treeCount) / 4.0F) * 8.0F);

        float lodBoxSize = lod1Size / 3.0F;
        const vec2 offsets[8] = vec2[8](
        vec2(-1.5F*lodBoxSize, 0.5F*lodBoxSize),
        vec2(-0.5F*lodBoxSize, 0.5F*lodBoxSize),
        vec2(0.5F*lodBoxSize, 0.5F*lodBoxSize),
        vec2(-1.5F*lodBoxSize, -0.5F*lodBoxSize),
        vec2(0.5F*lodBoxSize, -0.5F*lodBoxSize),
        vec2(-1.5F*lodBoxSize, -1.5F*lodBoxSize),
        vec2(-0.5F*lodBoxSize, -1.5F*lodBoxSize),
        vec2(0.5F*lodBoxSize, -1.5F*lodBoxSize)
        );

        pos *= lod1Size / 3.0F;
        pos += offsets[lodId];
    } else {
        // LOD 2
        int lodId = (invocationId - treeCount / 4 * 3) / 8;
        // 0.0F - 1000.0F
        pos *= gridWidth;
        // -500.0F - 500.0F
        pos -= gridWidth / 2.0F;

        // TODO remove this eventually
        ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
        imageStore(imgOutput, pixelCoords, vec4(0.0F, 0.0F, 0.0F, 1.0F));
        return;
    }

    vec4 noise = generateHeight(pos, noiseLayers, numNoiseLayers, useFiniteDifferences, finiteDifference, power, bowlStrength, platformHeight, seed);
    vec3 position = vec3(pos.x, noise.x, pos.y);

    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    imageStore(imgOutput, pixelCoords, vec4(position, 1.0F));
}
