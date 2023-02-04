#version 300 es
precision mediump float;

in vec2 UV;

uniform bool useTexture;
uniform vec3 flatColor;
uniform sampler2D textureSampler;

out vec4 color;

void main() {
    if (useTexture) {
        vec4 textureColor = texture(textureSampler, UV);
        color = vec4(flatColor * textureColor.r, 1.0F);
    } else {
        color = vec4(flatColor, 1.0F);
    }
}
