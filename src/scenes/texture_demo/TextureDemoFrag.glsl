#version 300 es
precision mediump float;

in vec2 UV;

uniform sampler2D textureSampler;

out vec4 color;

void main() {
    color = vec4(texture(textureSampler, UV).rgb, 1.0F);
}
