#version 130

attribute vec3 a_Position;

uniform mat4 u_Projection;

void main() {
    gl_Position = u_Projection * vec4(a_Position, 1.0);
}
