#version 330 core

#include "NoiseLib.glsl"

in vec3 a_Position;

out float height;
out float normalized_height;
flat out int instanceId;
flat out float colorFactor;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform int treeCount;

uniform NoiseLayer noiseLayers[MAX_NUM_NOISE_LAYERS];
uniform int numNoiseLayers;
uniform float finiteDifference;
uniform bool useFiniteDifferences;
uniform float power;
uniform float bowlStrength;
uniform float platformHeight;
uniform int seed;

void placementRandom(inout vec2 pos) {
    pos = vec2(gold_noise(pos.xx, float(gl_InstanceID)), gold_noise(pos.yy, float(gl_InstanceID)));
}

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
    int width = int(sqrt(treeCount));
    vec2 pos = vec2(float(gl_InstanceID%width)/width, float(gl_InstanceID/width)/width);

    placementRandom(pos);
    //    placementJiggleAroundPos(pos);

    pos *= 1000.0F;// width of the grid
    pos -= 500.0F;// half of the width of the grid

    vec4 noise = generateHeight(pos, noiseLayers, numNoiseLayers, useFiniteDifferences, finiteDifference, power, bowlStrength, platformHeight, seed);
    height = noise.x;
    normalized_height = noise.w;

    vec3 position = a_Position + vec3(pos.x, noise.x + 0.5F, pos.y);

    // TODO try to place tree 10 times and then discard, if still no valid position was found
    if (discardTree(pos, noise)) {
        colorFactor = 0.0F;
        //        position.y = 0.0F;
    } else {
        colorFactor = 1.0F;
    }
    colorFactor = 1.0F;

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0F);
    instanceId = gl_InstanceID;
}
