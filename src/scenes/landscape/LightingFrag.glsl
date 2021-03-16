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
uniform vec3 cameraDir;
uniform float aspectRatio;
uniform mat4 viewMatrix;
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

vec4 calculateLight(vec3 FragPos, vec3 Normal, Light light, vec4 Diffuse, float AmbientOcclusion) {
    vec4 ambient = vec4(vec4(light.Ambient * light.Color, 1.0F) * Diffuse * AmbientOcclusion);
    vec3 viewDir = normalize(-FragPos);// viewpos is (0.0.0)
    vec3 fragmentToLightDir = normalize(light.FragmentToLightDir);

    // diffuse
    vec4 diffuse = max(dot(Normal, fragmentToLightDir), 0.0) * Diffuse * vec4(light.Diffuse * light.Color, 1.0F);

    // specular
    vec3 halfwayDir = normalize(fragmentToLightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 2.0);
    vec3 specular = light.Specular * light.Color * spec;

    return ambient + diffuse + vec4(specular, 1.0F);
}

void main() {
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec4 Diffuse = texture(gAlbedo, TexCoords);

    if (FragPos.x == 0.0 && FragPos.y == 0.0 && FragPos.z == 0.0) {
        // position of the background, set this to something very far away
        vec2 uv = TexCoords - vec2(0.5);
        uv.x *= aspectRatio;
        FragPos = normalize(vec3(uv, -1.0)) * 1e10;
    }

    bool cloudLighting = texture(gDoLighting, TexCoords).r == 1.0F;
    //    if (cloudLighting) {
    //        color = vec4(1.0, 0.0, 0.0, 1.0);
    //        return;
    //    }

    float AmbientOcclusion = 1.0F;
    if (useAmbientOcclusion) {
        AmbientOcclusion = texture(ssao, TexCoords).r;
    }

    vec4 lighting = calculateLight(FragPos, Normal, light, Diffuse, AmbientOcclusion);

    if (useAtmosphericScattering) {
        #if 1
        if (cloudLighting) {
            vec2 uv = TexCoords - vec2(0.5);
            uv.x *= aspectRatio;
            FragPos = normalize(vec3(uv, -1.0)) * 1e10;
            vec4 prevLight = lighting;

            float lightPower = 50.0;
            lighting = calcScattering(
            cameraPosition,
            FragPos,
            normalize(-light.FragmentToLightDir),
            light.Color,
            lightPower,
            atmosphere,
            lighting);

            lighting = mix(lighting, prevLight, 0.3);
        } else {
            float lightPower = 50.0;
            lighting = calcScattering(
            cameraPosition,
            FragPos,
            normalize(-light.FragmentToLightDir),
            light.Color,
            lightPower,
            atmosphere,
            lighting);
        }
            #else
        float lightPower = 100.0;
        float distance = length(FragPos - cameraPosition);
        lighting.xyz = calculate_scattering(
        cameraPosition,
        normalize(cameraDir),
        distance,
        lighting.xyz,
        normalize(light.FragmentToLightDir),
        vec3(lightPower)
        );
        #endif
    }

    if (useACESFilm) {
        lighting = ACESFilm(lighting);
    }

    // Exposure tone mapping
    vec4 mapped = vec4(1.0) - exp(-lighting * exposure);

    // gamma correction
    lighting = pow(mapped, vec4(1.0 / gamma));

    color = lighting;

    //    color = vec4(FragPos, 1.0);
    //    vec3 cameraDir = FragPos - cameraPosition;
    //    color = vec4(length(cameraDir) / 1000, 0.0, 0.0, 1.0);
}
