#version 330 core

layout (location = 0) in vec3 position_in;

out vec3 position_cs_in;

void main() {
    position_cs_in = position_in;
}
