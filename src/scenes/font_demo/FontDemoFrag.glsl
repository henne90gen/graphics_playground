#version 330 core

in vec2 UV;

uniform bool useTexture = false;
uniform vec3 flatColor;
uniform sampler2D textureSampler;

out vec4 color;

void main() {
    if (useTexture) {
        vec4 textureColor = texture(textureSampler, UV);
        color = vec4(flatColor, textureColor.r);
    } else {
        color = vec4(flatColor, 1.0F);
    }
}
