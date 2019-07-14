#version 130

uniform sampler2D u_TextureSampler;
uniform sampler2D u_NormalSampler;

varying vec2 v_UV;
varying vec3 v_Normal;

void main() {
    gl_FragColor = texture2D(u_TextureSampler, v_UV);
}
