#version 440 core

layout (triangles, equal_spacing, cw) in;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    vec4 pos = (gl_TessCoord.x * gl_in[0].gl_Position) +
    (gl_TessCoord.y * gl_in[1].gl_Position) +
    (gl_TessCoord.z * gl_in[2].gl_Position);

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * pos;
}
