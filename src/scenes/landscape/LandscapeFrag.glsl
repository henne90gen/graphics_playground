#version 330 core

in vec2 uv_frag_in;
in vec3 normal_frag_in;
in vec3 model_position;
in float normalized_height;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedoSpec;
layout (location = 3) out vec3 gDoLighting;

uniform mat3 normalMatrix;

uniform float grassLevel;
uniform float rockLevel;
uniform float blur;

uniform sampler2D grassTexture;
uniform sampler2D dirtTexture;
uniform sampler2D rockTexture;

vec3 getSurfaceColor(float height) {
    // TODO make color dependent on normal
    const float grassDamper = 0.75F;
    const float dirtDamper = 0.9F;
    if (height < grassLevel-blur) {
        vec3 grassColor = texture(grassTexture, uv_frag_in).rgb * grassDamper;
        return grassColor;
    } else if (height < grassLevel+blur) {
        vec3 grassColor = texture(grassTexture, uv_frag_in).rgb * grassDamper;
        vec3 dirtColor = texture(dirtTexture, uv_frag_in).rgb * dirtDamper;
        float t = (height-(grassLevel-blur)) / (2.0F*blur);
        return mix(grassColor, dirtColor, t);
    } else if (height < rockLevel-blur){
        vec3 dirtColor = texture(dirtTexture, uv_frag_in).rgb * dirtDamper;
        return dirtColor;
    } else if (height < rockLevel+blur) {
        vec3 dirtColor = texture(dirtTexture, uv_frag_in).rgb * dirtDamper;
        vec3 rockColor = texture(rockTexture, uv_frag_in).rgb;
        float t = (height-(rockLevel-blur)) / (2.0F*blur);
        return mix(dirtColor, rockColor, t);
    } else {
        vec3 rockColor = texture(rockTexture, uv_frag_in).rgb;
        return rockColor;
    }
}

void main() {
    gPosition = model_position;
    gNormal = normalize(normalMatrix * normal_frag_in);
    gAlbedoSpec = getSurfaceColor(normalized_height);
}
