#version 330 core

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

uniform bool useAmbientOcclusion;

struct Light {
    vec3 Position;
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

        #if 0
    // Point Light
    vec3 ambient = vec3(0.3 * Diffuse * AmbientOcclusion);
    vec3 viewDir = normalize(-FragPos);// viewpos is (0.0.0)
    // diffuse
    vec3 lightDir = normalize(light.Position - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.Color;
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = light.Color * spec;
    // attenuation
    float distance = length(light.Position - FragPos);
    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);
    diffuse *= attenuation;
    specular *= attenuation;
    vec3 lighting = ambient;
    lighting += diffuse + specular;
    #else
    // Directional Light
    vec3 viewDir = normalize(-FragPos);// viewpos is vec3(0,0,0)
    vec3 lightDir = -sunDirection;
    // diffuse shading
    float diff = max(dot(Normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(lightDir, Normal);
    float shininess = 2.0F;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    float lightPower = 1.0F;
    vec3 lightColor = light.Color * lightPower;
    vec3 ambient = lightColor * 0.1F;
    vec3 diffuse = lightColor;
    vec3 specular = lightColor * 0.15F;

    ambient  *= vec3(0.3 * Diffuse * AmbientOcclusion);
    diffuse  *= diff * Diffuse;
    specular *= spec;

    vec3 lighting = vec3(0.0F);
    lighting += ambient;
    lighting += diffuse;
    lighting += specular;
    #endif

    color = vec4(lighting, 1.0);
}
