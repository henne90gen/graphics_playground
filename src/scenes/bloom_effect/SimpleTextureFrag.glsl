#version 300 es
precision mediump float;

in vec2 UV;

uniform sampler2D u_Texture;
uniform float u_ColorAmplifier;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

void main() {
    FragColor = texture(u_Texture, UV) * u_ColorAmplifier;
    BrightColor = FragColor;
}
