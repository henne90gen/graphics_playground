#version 300 es
precision mediump float;

in float h;

out vec4 color;

void main() {
    float c = h*0.7F + 0.3F;
    color = vec4(c, c, c, 1.0F);
}
