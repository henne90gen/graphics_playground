#version 330 core

in vec2 v_UV;

uniform sampler2D u_TextureSampler;
uniform bool u_RenderCanvas;

out vec4 color;

void main() {
    if (u_RenderCanvas) {
        color = vec4(texture(u_TextureSampler, v_UV).rgb, 1.0);
    } else {
        color = vec4(1.0, 1.0, 1.0, 1.0);
    }
}
