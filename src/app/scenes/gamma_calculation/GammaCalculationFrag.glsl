#version 330 core

in vec2 UV;

uniform float grayValue;
uniform bool shouldRenderTexture;
uniform sampler2D textureSampler;

out vec4 color;

void main() {
    if (shouldRenderTexture) {
        color = vec4(texture(textureSampler, UV).rgb, 1.0);
    } else {
        color = vec4(grayValue, grayValue, grayValue, 1.0);
    }
}
