#version 330 core

in vec2 UV;

out vec4 FragColor;

uniform sampler2D u_Scene;
uniform sampler2D u_BloomBlur;

uniform float u_Exposure;
uniform bool u_Bloom;

void main() {
    const float gamma = 2.2;
    vec3 hdrColor = texture(u_Scene, UV).rgb;

    if (u_Bloom) {
        vec3 bloomColor = texture(u_BloomBlur, UV).rgb;
        // additive blending
        hdrColor += bloomColor;
    }

    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * u_Exposure);

    // also gamma correct while we're at it
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}
