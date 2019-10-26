#version 130

varying vec2 v_UV;

uniform bool u_UseTexture;
uniform sampler2D u_TextureSampler;
uniform vec3 u_Color;

void main() {
    if (u_UseTexture) {
        gl_FragColor = texture(u_TextureSampler, v_UV);
    } else {
        gl_FragColor = vec4(u_Color, 1.0);
    }
}
