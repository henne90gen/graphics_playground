#version 300 es
precision mediump float;

in vec2 TexCoords;

uniform sampler2D textureId;

out vec4 color;

void main() {
    color = texture(textureId, TexCoords);
}
