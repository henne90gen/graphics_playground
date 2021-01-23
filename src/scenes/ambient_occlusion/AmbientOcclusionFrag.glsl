#version 330 core

in vec2 uv;

uniform bool useTexture;
uniform sampler2D frameTexture;

out vec4 color;

void main() {
    if (useTexture) {
        color = texture(frameTexture, uv);
    } else {
        color = vec4(1.0, 1.0, 1.0, 1.0);
    }
}
