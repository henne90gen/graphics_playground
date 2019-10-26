#version 130

attribute vec3 a_Position;
attribute vec2 a_UV;

varying vec2 v_UV;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
    v_UV = a_UV;
}
