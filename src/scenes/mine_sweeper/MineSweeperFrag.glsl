#version 300 es
precision mediump float;

in vec2 UV;

uniform bool useTexture;
uniform vec3 flatColor;
uniform sampler2D textureSampler;

out vec4 color;

void main() {
    if (useTexture) {
        if (UV.x < 0.0 || UV.x > 1.0 || UV.y < 0.0 || UV.y > 1.0) {
            discard;
        }
        vec4 textureColor = texture(textureSampler, UV);
        color = vec4(flatColor, textureColor.r);
        // color = vec4(UV, 0, 1);
    } else {
        color = vec4(flatColor, 1.0F);
    }
}
