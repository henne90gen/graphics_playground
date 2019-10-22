#version 130

varying vec2 v_UV;

uniform sampler2D u_TextureSampler;

void main() {
    gl_FragColor = texture(u_TextureSampler, v_UV);
}
