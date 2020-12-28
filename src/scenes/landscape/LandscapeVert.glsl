#version 330 core

layout (location = 0) in vec2 position_in;

out vec2 position_tcs_in;

uniform mat4 viewMatrix;

void main() {
    position_tcs_in = position_in;
}
