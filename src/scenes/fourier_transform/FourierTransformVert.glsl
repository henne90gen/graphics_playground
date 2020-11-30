#version 330 core

in vec2 a_Position;
in vec2 a_UV;

uniform bool u_RenderCanvas;
uniform mat4 u_View;

out vec2 v_UV;

void main() {
    if (u_RenderCanvas) {
        v_UV = a_UV;
    }
    gl_Position = u_View * vec4(a_Position, 0.0, 1.0);
}
