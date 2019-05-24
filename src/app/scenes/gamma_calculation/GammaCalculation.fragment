#version 130

varying vec2 UV;

uniform float grayValue;
uniform bool shouldRenderTexture;
uniform sampler2D textureSampler;

void main() {
    if (shouldRenderTexture) {
        gl_FragColor = vec4(texture(textureSampler, UV).rgb, 1.0);
    } else {
        gl_FragColor = vec4(grayValue, grayValue, grayValue, 1.0);
    }
}
