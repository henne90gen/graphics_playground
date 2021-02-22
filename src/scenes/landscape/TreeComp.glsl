#version 430 core

#include "NoiseLib.glsl"

layout (local_size_x = 1, local_size_y = 1) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;

uniform float lodSize = 1000.0F;
uniform float lodInnerSize = 100.0F;
uniform float lod1Size;
uniform float lod2Size;
uniform int treeCount;

uniform NoiseLayer noiseLayers[MAX_NUM_NOISE_LAYERS];
uniform int numNoiseLayers;
uniform float finiteDifference;
uniform bool useFiniteDifferences;
uniform float power;
uniform float bowlStrength;
uniform float platformHeight;
uniform int seed;

const int MAX_NUM_POSITIONS = 40;
vec3 positions[MAX_NUM_POSITIONS];

vec2 placementRandom(float seed) {
    vec2 pos = vec2(seed, seed);
    pos = vec2(snoise2(pos*pos+pos).x, snoise2(pos+pos).x);
    pos += 1.0F;
    pos /= 2.0F;

    // TODO 'pos' is not strictly between 0 and 1 at this point, thus correcting it a bit
    pos -= 0.05F;

    return pos;
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
    //    if (invocationId > treeCount) {
    //        // don't calculate positions that won't be used
    //        ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    //        imageStore(imgOutput, pixelCoords, vec4(0.0F, 0.0F, 0.0F, 1.0F));
    //        return;
    //    }

    const float lodH = lodSize / 2.0F;
    const float lodIH = lodInnerSize / 2.0F;
    const float longEdge = lodH + lodIH;
    const float shortEdge = lodH - lodIH;
    const float longEdgeH = longEdge / 2.0F;
    const float shortEdgeH = shortEdge / 2.0F;
    const vec2 scales[2] = vec2[2](
    vec2(longEdgeH, shortEdgeH),
    vec2(shortEdgeH, longEdgeH)
    );

    const vec2 offsets[4][4] = vec2[4][4](
    vec2[4](vec2(-lodH, lodIH+shortEdgeH), vec2(-lodH+longEdgeH, lodIH+shortEdgeH), vec2(-lodH, lodIH), vec2(-lodH+longEdgeH, lodIH)),
    vec2[4](vec2(lodIH, lodH-longEdgeH), vec2(lodIH+shortEdgeH, lodH-longEdgeH), vec2(lodIH, -lodIH), vec2(lodIH+shortEdgeH, -lodIH)),
    vec2[4](vec2(-lodH, lodIH-longEdgeH), vec2(-lodIH-shortEdgeH, lodIH-longEdgeH), vec2(-lodH, -lodH), vec2(-lodH+shortEdgeH, -lodH)),
    vec2[4](vec2(-lodIH, -lodIH-shortEdgeH), vec2(-lodIH + longEdgeH, -lodIH-shortEdgeH), vec2(-lodIH, -lodH), vec2(-lodIH + longEdgeH, -lodH))
    );

    const int invocationId = int(4*gl_GlobalInvocationID.x + gl_GlobalInvocationID.y);
    const int numPositions = 64;
    for (int i = 0; i < numPositions; i++) {
        vec3 position;
        int count = 0;
        do {
            count++;

            float s = (
            float(gl_GlobalInvocationID.x+1)*float(gl_GlobalInvocationID.x+1)*float(gl_GlobalInvocationID.x+1) +
            float(gl_GlobalInvocationID.y)*float(gl_GlobalInvocationID.y) +
            float(i)) / 100 +
            float(seed) / 100 +
            float(count) * 5.0F;

            vec2 pos = placementRandom(s);

            pos *= scales[gl_GlobalInvocationID.x % 2];
            pos += offsets[gl_GlobalInvocationID.x][gl_GlobalInvocationID.y];

            #define INSPECT_BATCH 0
            #if INSPECT_BATCH
            if (gl_GlobalInvocationID.x != 0 || gl_GlobalInvocationID.y != 0) {
                pos += vec2(lodSize, lodSize);
                position = vec3(pos.x, 0, pos.y);
                break;
            }
                #endif

            vec4 noise = generateHeight(pos, noiseLayers, numNoiseLayers, useFiniteDifferences, finiteDifference, power, bowlStrength, platformHeight, seed);
            position = vec3(pos.x, noise.x, pos.y);

            float minDistance = 20.0F;
            bool tooClose = false;
            for (int j = 0; j < i; j++) {
                int positionId = numPositions*invocationId + j;
                int col = positionId % imgSize.x;
                int row = positionId / imgSize.x;
                ivec2 pixelCoords = ivec2(col, row);
                vec3 p = imageLoad(imgOutput, pixelCoords).xyz;
                if (length(p-position) < minDistance) {
                    tooClose = true;
                    break;
                }
            }
            if (!tooClose) {
                break;
            }
        } while (count < 100);

        int positionId = numPositions*invocationId + i;
        int col = positionId % imgSize.x;
        int row = positionId / imgSize.x;
        ivec2 pixelCoords = ivec2(col, row);
        imageStore(imgOutput, pixelCoords, vec4(position, 1.0F));
    }
}
