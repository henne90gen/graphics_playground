#version 330 core

#include "ScatterLib.glsl"

#define USE_SIMPLE_ATMOSPHERIC_SCATTERING 0

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

uniform vec4 cameraOrientation;
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

vec3 calculateLight(vec3 FragPos, vec3 Normal, Light light, vec3 Diffuse, float AmbientOcclusion) {
    // TODO this produces a shine through artifact, when the light comes from behind the fragment

    vec3 ambient = light.Ambient * light.Color * Diffuse * AmbientOcclusion;
    vec3 viewDir = normalize(-FragPos);// camera is at vec3(0,0,0)
    vec3 fragmentToLightDir = normalize(light.FragmentToLightDir);

    // diffuse
    vec3 diffuse = max(dot(Normal, fragmentToLightDir), 0.0) * Diffuse * light.Diffuse * light.Color;

    // specular
    vec3 halfwayDir = normalize(fragmentToLightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 2.0);
    vec3 specular = light.Specular * light.Color * spec;

    return ambient + diffuse + specular;
}

void main() {
    vec3 FragPos = texture(gPosition, TexCoords).xyz;
    vec3 Normal = normalize(texture(gNormal, TexCoords).xyz);
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;

    bool cloudLighting = texture(gDoLighting, TexCoords).r == 1.0F;

    float AmbientOcclusion = 1.0F;
    if (useAmbientOcclusion) {
        AmbientOcclusion = texture(ssao, TexCoords).r;
    }

    if (FragPos.x == 0.0 && FragPos.y == 0.0 && FragPos.z == 0.0) {
        // position of the background, set this to something very far away
        vec2 uv = (TexCoords - vec2(0.5)) * 2.0;
        uv.x *= aspectRatio;
        FragPos = normalize(vec3(uv, -1.0)) * 1e12;
    }

    vec3 lighting = vec3(0.0);
    if (cloudLighting) {
        lighting = calculateLight(FragPos, Normal, light, vec3(0.0), AmbientOcclusion);
    } else {
        lighting = calculateLight(FragPos, Normal, light, Diffuse, AmbientOcclusion);
    }

    if (useAtmosphericScattering) {
        #if USE_SIMPLE_ATMOSPHERIC_SCATTERING
        if (cloudLighting) {
            vec2 uv = (TexCoords - vec2(0.5)) * 2.0;
            uv.x *= aspectRatio;
            FragPos = normalize(vec3(uv, -1.0)) * 1e12;
            vec3 cloudLight = lighting;

            float lightPower = 50.0;
            lighting = calcScattering(
            cameraPosition,
            FragPos,
            normalize(-light.FragmentToLightDir),
            light.Color,
            lightPower,
            atmosphere,
            vec4(lighting, 1.0)).rgb;

            lighting = mix(lighting, cloudLight, 0.3);
        } else {
            float lightPower = 50.0;
            lighting = calcScattering(
            vec3(0.0),
            FragPos,
            normalize(-light.FragmentToLightDir),
            light.Color,
            lightPower,
            atmosphere,
            vec4(lighting, 1.0)).rgb;
        }
            #else
        if (cloudLighting) {
            vec2 uv = (TexCoords - vec2(0.5)) * 2.0;
            uv.x *= aspectRatio;
            FragPos = normalize(vec3(uv, -1.0)) * 1e12;
        }

        float lightPower = 30.0;
        vec3 cDir = FragPos;
        // TODO use atmosphere settings for this scattering function as well
        lighting.rgb = calculate_scattering(
        vec3(0.0),
        normalize(cDir),
        length(cDir),
        cameraOrientation,
        lighting.rgb,
        normalize(light.FragmentToLightDir),
        vec3(lightPower));

        if (cloudLighting){
            lighting = mix(lighting, vec3(1.0F), Diffuse.r);
        }
            #endif
    }

    if (useACESFilm) {
        lighting = ACESFilm(vec4(lighting, 1.0)).rgb;
    }

    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-lighting * exposure);

    // gamma correction
    lighting = pow(mapped, vec3(1.0 / gamma));

    color = vec4(lighting, 1.0);

    //    color = vec4(FragPos, 1.0);
    //    vec3 cameraDir = FragPos - cameraPosition;
    //    color = vec4(length(cameraDir) / 1000, 0.0, 0.0, 1.0);
}
