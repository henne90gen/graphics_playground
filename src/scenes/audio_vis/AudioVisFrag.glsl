#version 330 core

in float h;

out vec4 color;

void main() {
    float c = h*0.7 + 0.3;
    color = vec4(c, c, c, 1.0);
}
