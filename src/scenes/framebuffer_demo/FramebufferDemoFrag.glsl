#version 300 es
precision mediump float;

in vec2 v_UV;

uniform bool u_UseTexture;
uniform sampler2D u_Texture;
uniform vec3 u_Color;

out vec4 color;

void main() {
    if (u_UseTexture) {
        color = texture(u_Texture, vec2(v_UV.x * -1.0, v_UV.y));
    } else {
        color = vec4(u_Color, 1.0);
    }
}
