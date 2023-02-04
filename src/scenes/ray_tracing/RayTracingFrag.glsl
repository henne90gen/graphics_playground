#version 300 es
precision mediump float;

in vec2 v_UV;

uniform bool u_UseTexture;
uniform sampler2D u_TextureSampler;
uniform vec3 u_Color;

out vec4 color;

void main() {
    if (u_UseTexture) {
        color = texture(u_TextureSampler, v_UV);
    } else {
        color = vec4(u_Color, 1.0F);
    }
}
