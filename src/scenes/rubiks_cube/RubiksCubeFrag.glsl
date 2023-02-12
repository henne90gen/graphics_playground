#version 300 es
precision mediump float;

in vec2 vUV;
in vec3 vColor;

out vec4 color;

void main() {
    color = vec4(vColor, 1.0F);
    if (vUV.x < 0.01F) {
        color = vec4(0.0F);
    } else if(vUV.x > 0.99F) {
        color = vec4(0.0F);
    }
    if (vUV.y < 0.01F) {
        color = vec4(0.0F);
    } else if(vUV.y > 0.99F) {
        color = vec4(0.0F);
    }
}
