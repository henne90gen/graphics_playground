#version 130

varying vec2 v_UV;

uniform sampler2D u_TextureSampler;
uniform bool u_RenderCanvas;

void main() {
    if (u_RenderCanvas) {
        gl_FragColor = texture(u_TextureSampler, v_UV);
    } else {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
}
