#version 330

in vec2 UV;

uniform sampler2D textureSampler;

out vec4 color;

void main() {
    vec4 c = texture(textureSampler, UV);
    color = vec4(c.r, c.r, c.r, 1.0);
//    color = vec4(1.0F, 1.0F, 1.0F, 1.0F);
}
