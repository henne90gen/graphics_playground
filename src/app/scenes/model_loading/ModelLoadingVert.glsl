#version 130

attribute vec3 a_Position;
attribute vec3 a_Normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

varying vec3 v_Color;

void main() {
    v_Color = a_Normal;
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
}
