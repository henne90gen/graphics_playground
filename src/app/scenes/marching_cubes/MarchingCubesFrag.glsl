#version 130

uniform vec3 u_Dimensions;

varying vec3 v_FinalPosition;

void main() {
    vec3 color = v_FinalPosition / u_Dimensions;
    gl_FragColor = vec4(color, 1.0);
}
