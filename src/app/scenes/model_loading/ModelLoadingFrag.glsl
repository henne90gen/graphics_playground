#version 130

varying vec3 v_Normal;
varying vec2 v_UV;

uniform sampler2D u_TextureSampler;

uniform bool u_HasNormals;
uniform bool u_HasTexture;

void main() {
    if (u_HasNormals && !u_HasTexture) {
        gl_FragColor = vec4(v_Normal, 1.0);
    } else if (u_HasNormals && u_HasTexture) {
        gl_FragColor = vec4(texture(u_TextureSampler, v_UV).rgb, 1.0);
    } else {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
}
