#version 330 core

in vec2 TexCoords;

uniform sampler2D textureId;

out vec4 color;

void main() {
    color = texture(textureId, TexCoords);
}
