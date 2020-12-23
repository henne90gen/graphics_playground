#version 440 core

layout (vertices = 3) out;

uniform float innerTess;
uniform vec3 outerTess;

in vec3 position_cs_in[];

out vec3 position_es_in[];

void main() {
    gl_TessLevelInner[0] = innerTess;

    gl_TessLevelOuter[0] = outerTess.x;
    gl_TessLevelOuter[1] = outerTess.y;
    gl_TessLevelOuter[2] = outerTess.z;

    position_es_in[gl_InvocationID] = position_cs_in[gl_InvocationID];
}
