#version 330 core

layout (location = 0) in vec3 position_in;

out vec3 position_tcs_in;

void main() {
    position_tcs_in = position_in;
}
