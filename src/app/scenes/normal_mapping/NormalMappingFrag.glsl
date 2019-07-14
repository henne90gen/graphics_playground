#version 130

uniform sampler2D u_TextureSampler;
uniform sampler2D u_NormalSampler;

varying vec2 v_UV;

void main() {
    gl_FragColor = texture2D(u_TextureSampler, v_UV);
}
