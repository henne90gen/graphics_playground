#version 330 core

in vec3 v_Normal;
in vec2 v_UV;

uniform sampler2D u_TextureSampler;
uniform bool u_HasNormals;
uniform bool u_HasTexture;

out vec4 color;

void main() {
    if (u_HasNormals && !u_HasTexture) {
        color = vec4(v_Normal, 1.0);
    } else if (u_HasNormals && u_HasTexture) {
        color = vec4(texture(u_TextureSampler, v_UV).rgb, 1.0);
    } else {
        color = vec4(1.0, 1.0, 1.0, 1.0);
    }
}
