#version 130

attribute vec2 a_Position;

uniform int u_DrawMode;
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    vec4 position = vec4(a_Position, 0.0, 1.0);
    if (u_DrawMode == 0) {
        position = u_Model * position;
    }
    gl_Position = u_Projection * u_View * position;
}
