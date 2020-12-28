#version 440 core

layout (vertices = 3) out;

uniform float innerTess;
uniform vec3 outerTess;

in vec3 position_tcs_in[];

out vec3 position_tes_in[];

void main() {
    gl_TessLevelInner[0] = innerTess;

    gl_TessLevelOuter[0] = outerTess.x;
    gl_TessLevelOuter[1] = outerTess.y;
    gl_TessLevelOuter[2] = outerTess.z;

    position_tes_in[gl_InvocationID] = position_tcs_in[gl_InvocationID];
}
