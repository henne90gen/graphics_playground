#version 130

varying vec2 v_UV;

uniform sampler2D u_TextureSampler;
uniform bool u_RenderCanvas;

void main() {
    if (u_RenderCanvas) {
        gl_FragColor = vec4(texture(u_TextureSampler, v_UV).rgb, 1.0);
    } else {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
}
