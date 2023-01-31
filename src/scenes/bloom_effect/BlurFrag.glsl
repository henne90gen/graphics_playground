#version 300 es
precision mediump float;

out vec4 FragColor;

in vec2 UV;

uniform sampler2D image;

uniform bool horizontal;

const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    // get size of single texel
    vec2 tex_offset = 1.0 / vec2(textureSize(image, 0));

    // current fragment's contribution
    vec3 result = texture(image, UV).rgb * weight[0];

    if (horizontal) {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(image, UV + vec2(tex_offset.x * float(i), 0.0F)).rgb * weight[i];
            result += texture(image, UV - vec2(tex_offset.x * float(i), 0.0F)).rgb * weight[i];
        }
    } else {
        for (int i = 1; i < 5; ++i) {
            result += texture(image, UV + vec2(0.0F, tex_offset.y * float(i))).rgb * weight[i];
            result += texture(image, UV - vec2(0.0F, tex_offset.y * float(i))).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0F);
}
