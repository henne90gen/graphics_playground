#version 300 es
precision mediump float;

in vec3 a_Position;
in vec2 a_UV;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 v_UV;

void main() {
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0F);
    v_UV = a_UV;
}
