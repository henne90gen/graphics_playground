#version 330 core

in vec2 uv;

uniform sampler2D textureId;

out vec4 color;

void main() {
    color = texture(textureId, uv);
}
