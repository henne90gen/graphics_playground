#version 330 core

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

uniform bool useAmbientOcclusion;
uniform bool usePointLight;

struct Light {
    vec4 Position;
    vec3 Color;

    float Linear;
    float Quadratic;
};
uniform Light light;
uniform vec3 sunDirection;

out vec4 color;

void main() {
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
    float AmbientOcclusion = 1.0F;
    if (useAmbientOcclusion) {
        AmbientOcclusion = texture(ssao, TexCoords).r;
    }

    vec3 ambient = vec3(0.3 * Diffuse * AmbientOcclusion);
    vec3 viewDir = normalize(-FragPos);// viewpos is (0.0.0)
    vec3 lightDir = vec3(0.0F);
    if (light.Position.w == 0.0F) {
        lightDir = light.Position.xyz;
    } else {
        lightDir = normalize(light.Position.xyz - FragPos);
    }

    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.Color;

    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = light.Color * spec;

    if (light.Position.w == 1.0F) {
        // attenuation
        float distance = length(light.Position.xyz - FragPos);
        float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
    }

    vec3 lighting = ambient + diffuse + specular;
    color = vec4(lighting, 1.0);
}
