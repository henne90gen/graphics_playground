#version 330 core

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gExtinction;
uniform sampler2D gInScatter;
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

out vec4 color;

// https://www.shadertoy.com/view/WlSSzK
vec3 ACESFilm(vec3 x) {
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
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
    float AmbientOcclusion = 1.0F;
    if (useAmbientOcclusion) {
        AmbientOcclusion = texture(ssao, TexCoords).r;
    }

    vec3 ambient = vec3(light.Ambient * light.Color * Diffuse * AmbientOcclusion);
    vec3 viewDir = normalize(-FragPos);// viewpos is (0.0.0)
    vec3 fragmentToLightDir = normalize(light.FragmentToLightDir);

    // diffuse
    vec3 diffuse = max(dot(Normal, fragmentToLightDir), 0.0) * Diffuse * light.Diffuse * light.Color;

    // specular
    vec3 halfwayDir = normalize(fragmentToLightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 2.0);
    vec3 specular = light.Specular * light.Color * spec;

    vec3 lighting = ambient + diffuse + specular;

    if (useAtmosphericScattering) {
        lighting *= texture(gExtinction, TexCoords).rgb;
        lighting += texture(gInScatter, TexCoords).rgb;
    }

    if (useACESFilm) {
        lighting = ACESFilm(lighting);
    }

    color = vec4(lighting, 1.0);
}
