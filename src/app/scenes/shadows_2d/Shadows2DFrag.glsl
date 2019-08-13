#version 130

uniform int u_DrawMode;

void main() {
    if (u_DrawMode == 0) {
        // light source
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    } else if (u_DrawMode == 1) {
        // walls
        gl_FragColor = vec4(0.5, 0.5, 0.5, 1.0);
    } else if (u_DrawMode == 2) {
        // rays
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
}
