#version 300 es
precision mediump float;

in vec2 v_UV;

uniform sampler2D u_TextureSampler;
uniform bool u_RenderCanvas;

out vec4 color;

void main() {
    if (u_RenderCanvas) {
        color = texture(u_TextureSampler, v_UV);
    } else {
        color = vec4(0.0, 0.0, 0.0, 0.0);
    }
}
