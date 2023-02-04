#version 300 es
precision mediump float;

in vec3 v_FinalPosition;

uniform vec3 u_Dimensions;

out vec4 color;

void main() {
    vec3 colorv3 = v_FinalPosition / u_Dimensions;
    color = vec4(colorv3, 1.0F);
}
