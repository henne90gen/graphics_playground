#version 330 core

in vec2 UV;

uniform sampler2D textureSampler;
uniform vec3 textColor;

out vec4 color;

void main() {
    vec4 textureColor = texture(textureSampler, UV);
    color = vec4(textColor, textureColor.r);
}
