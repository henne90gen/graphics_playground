#version 330 core

in vec2 UV;

uniform sampler2D textureSampler;
uniform bool isGrayScale;

out vec4 color;

void main() {
    vec4 textureColor = texture(textureSampler, UV);
    if (isGrayScale) {
        color = vec4(textureColor.r);
        color.a = 1.0F;
    } else {
        color = textureColor;
    }
}
