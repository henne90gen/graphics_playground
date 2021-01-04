#version 440 core

layout (vertices = 3) out;

in vec2 position_tcs_in[];

uniform float tessellation;

out vec2 position_tes_in[];

void main() {
    // TODO calculate tessellation with distance to camera
    gl_TessLevelInner[0] = tessellation;

    gl_TessLevelOuter[0] = tessellation;
    gl_TessLevelOuter[1] = tessellation;
    gl_TessLevelOuter[2] = tessellation;

    position_tes_in[gl_InvocationID] = position_tcs_in[gl_InvocationID];
}
