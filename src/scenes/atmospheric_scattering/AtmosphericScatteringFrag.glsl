#version 330 core

struct DirLight {
    vec3 direction;
    vec3 color;
    float power;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float power;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

in vec3 normal_frag_in;
in vec3 model_position;
in float normalized_height;

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform vec3 cameraPosition;

out vec4 color;

const vec3 lightPosition = vec3(0.0F, 150.0F, 0.0F);
const vec3 lightDirection = vec3(0.0F, 150.0F, 0.0F);
const vec3 lightColor = vec3(1.0F);
const float lightPower = 1.0F;

const float grassLevel = 0.4F;
const float rockLevel = 1.0F;
const float blur = 0.05F;

const vec3 grassColor = vec3(19.0F/255.0F, 133.0F/255.0F, 16.0F/255.0F);
const vec3 rockColor = vec3(73.0F/255.0F, 60.0F/255.0F, 60.0F/255.0F);
const vec3 snowColor = vec3(255.0F/255.0F, 250.0F/255.0F, 250.0F/255.0F);

vec3 getSurfaceColor(float height) {
    const float grassDamper = 0.75F;
    if (height < grassLevel-blur) {
        return grassColor;
    } else if (height < grassLevel+blur) {
        float t = (height-(grassLevel-blur)) / (2.0F*blur);
        return mix(grassColor, rockColor, t);
    } else if (height < rockLevel-blur){
        return rockColor;
    } else if (height < rockLevel+blur) {
        float t = (height-(rockLevel-blur)) / (2.0F*blur);
        return mix(rockColor, snowColor, t);
    } else {
        return snowColor;
    }
}

vec4 calcDirLight(DirLight light, Material material, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 color = light.color * light.power;
    vec3 ambient = color * 0.1F;
    vec3 diffuse = color;
    vec3 specular = color * 0.15F;

    ambient  *= material.ambient;
    diffuse  *= diff * material.diffuse;
    specular *= spec * material.specular;

    vec3 result = vec3(0.0F);
    result += ambient;
    result += diffuse;
    result += specular;
    return vec4(result, 1.0F);
}

void main() {
    vec3 normal = normalize(normalMatrix * normal_frag_in);
    vec3 viewDir = normalize(cameraPosition - model_position);

    vec3 materialDiffuseColor = getSurfaceColor(normalized_height);
    vec3 materialAmbientColor = vec3(0.1, 0.1, 0.1) * materialDiffuseColor;
    vec3 materialSpecularColor = vec3(0.3, 0.3, 0.3);
    Material material = Material(materialAmbientColor, materialDiffuseColor, materialSpecularColor, 2);
    PointLight light = PointLight(lightPosition, lightColor, lightPower);
    DirLight dirLight = DirLight(lightDirection, lightColor, lightPower);

    color = vec4(0.0F);
    //    color += calcPointLightSimple(light, material, normal, viewDir);
    color += calcDirLight(dirLight, material, normal, viewDir);
}
