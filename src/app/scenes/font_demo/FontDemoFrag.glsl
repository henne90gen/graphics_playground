#version 130

varying vec2 UV;
uniform sampler2D textureSampler;
uniform vec3 textColor;

void main() {
    vec4 textureColor = texture(textureSampler, UV);
    float alpha = textureColor.r;
    vec4 sampled = vec4(1.0, 1.0, 1.0, alpha);
    gl_FragColor = vec4(textColor, 1.0) * sampled;
}
