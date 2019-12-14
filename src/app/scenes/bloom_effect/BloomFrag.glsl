#version 330 core

in vec2 UV;

out vec4 FragColor;

uniform sampler2D u_SceneTexture;
uniform sampler2D u_BloomBlurTexture;

uniform bool u_DoBloom = true;
uniform bool u_DoGammaCorrection = true;
uniform float u_Exposure;

void main() {
    const float gamma = 2.2;
    vec3 hdrColor = texture(u_SceneTexture, UV).rgb;

    if (u_DoBloom) {
        vec3 bloomColor = texture(u_BloomBlurTexture, UV).rgb;
        // additive blending
        hdrColor += bloomColor;
    }

    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * u_Exposure);

    // also gamma correct while we're at it
    if (u_DoGammaCorrection) {
        result = pow(result, vec3(1.0 / gamma));
    }
    FragColor = vec4(result, 1.0);
}
