#version 330 core

#include "ScatterLib.glsl"

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gDoLighting;
uniform sampler2D ssao;

uniform bool useAmbientOcclusion;
uniform bool useAtmosphericScattering;
uniform bool useACESFilm;

struct Light {
    vec3 FragmentToLightDir;
    vec3 Color;

    float Ambient;
    float Diffuse;
    float Specular;
};
uniform Light light;

uniform vec3 cameraPosition;
uniform vec3 atmosphere;

uniform float exposure = 1.0F;
uniform float gamma = 1.0F;

out vec4 color;

// https://www.shadertoy.com/view/WlSSzK
vec4 ACESFilm(vec4 x) {
    #if 0
    // original values
    float tA = 2.51;
    float tB = 0.03;
    float tC = 2.43;
    float tD = 0.59;
    float tE = 0.14;
    #else
    float tA = 3.01;
    float tB = 0.03;
    float tC = 2.43;
    float tD = 0.2;
    float tE = 0.8;
    #endif
    return clamp((x*(tA*x+tB))/(x*(tC*x+tD)+tE), 0.0, 1.0);
}

void main() {
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec4 Diffuse = texture(gAlbedo, TexCoords);

    // TODO turn on lighting for the sky as well
    if (texture(gDoLighting, TexCoords).r == 0.0F) {
        color = vec4(Diffuse.xyz, 1.0F);
        return;
    }

    float AmbientOcclusion = 1.0F;
    if (useAmbientOcclusion) {
        AmbientOcclusion = texture(ssao, TexCoords).r;
    }

    vec4 ambient = vec4(vec4(light.Ambient * light.Color, 1.0F) * Diffuse * AmbientOcclusion);
    vec3 viewDir = normalize(-FragPos);// viewpos is (0.0.0)
    vec3 fragmentToLightDir = normalize(light.FragmentToLightDir);

    // diffuse
    vec4 diffuse = max(dot(Normal, fragmentToLightDir), 0.0) * Diffuse * vec4(light.Diffuse * light.Color, 1.0F);

    // specular
    vec3 halfwayDir = normalize(fragmentToLightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 2.0);
    vec3 specular = light.Specular * light.Color * spec;

    vec4 lighting = ambient + diffuse + vec4(specular, 1.0F);

    if (useAtmosphericScattering) {
        vec3 extinction;
        vec3 inScatter;
        float lightPower = 200;
        calcScattering(cameraPosition, FragPos, -light.FragmentToLightDir, light.Color, lightPower, atmosphere, extinction, inScatter);
        lighting *= vec4(extinction, 1.0F);
        lighting += vec4(inScatter, 1.0F);
    }

    if (useACESFilm) {
        lighting = ACESFilm(lighting);
    }

    // Exposure tone mapping
    vec4 mapped = vec4(1.0) - exp(-lighting * exposure);

    // gamma correction
    lighting = pow(mapped, vec4(1.0 / gamma));

    color = lighting;
}
