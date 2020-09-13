#version 330 core

in vec2 UV;

uniform sampler2D textureSampler;

out vec4 color;

void main() {
    color = texture(textureSampler, UV);
//    color = vec4(1.0, 1.0, 1.0, 1.0);
}
