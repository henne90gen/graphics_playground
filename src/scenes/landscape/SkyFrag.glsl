#version 330 core

in vec3 position_frag_in;

uniform vec3 flatColor;

out vec4 color;

void main() {
    float height = -position_frag_in.y + 1.4F;
    color = vec4(flatColor * height, 1.0F);
}
