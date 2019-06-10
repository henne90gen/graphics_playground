#version 130

varying vec2 UV;
uniform sampler2D textureSampler;
uniform vec3 textColor;

void main() {
    vec4 textureColor = texture(textureSampler, UV);
    gl_FragColor = vec4(textColor, textureColor.r);
}
