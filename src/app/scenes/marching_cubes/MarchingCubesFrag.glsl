#version 130

uniform vec3 dimensions;

varying vec3 finalPosition;

void main() {
    vec3 color = finalPosition / dimensions;
    gl_FragColor = vec4(color, 1.0);
}
