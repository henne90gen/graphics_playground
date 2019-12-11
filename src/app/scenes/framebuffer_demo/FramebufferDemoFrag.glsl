#version 130

varying vec2 v_UV;

uniform bool u_UseTexture;
uniform sampler2D u_Texture;
uniform vec3 u_Color;

void main() {
    if (u_UseTexture) {
        gl_FragColor = texture(u_Texture, vec2(v_UV.x * -1.0, v_UV.y));
    } else {
        gl_FragColor = vec4(u_Color, 1.0);
    }
}
