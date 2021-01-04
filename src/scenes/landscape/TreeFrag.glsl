#version 330 core

in float height;
in float normalized_height;
flat in int instanceId;
flat in float colorFactor;

out vec4 color;

uniform int treeCount;

void main() {
    color = vec4(1.0F, 0.0F, 0.0F, 1.0F);
//    color = vec4(float(instanceId) / float(treeCount), 0.0F, 0.0F, 1.0F);
    color *= colorFactor;
}
