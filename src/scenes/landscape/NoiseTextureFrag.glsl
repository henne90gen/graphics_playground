#version 330

in vec2 UV;

uniform sampler2D textureSampler;
uniform int numChannels = 1;

out vec4 color;

void main() {
    vec4 c = texture(textureSampler, UV);
    if (numChannels == 1) {
        color = vec4(c.r, c.r, c.r, 1.0);
    } else if (numChannels == 3) {
        color = vec4(c.rgb, 1.0);
    } else {
        color = c;
    }
}
