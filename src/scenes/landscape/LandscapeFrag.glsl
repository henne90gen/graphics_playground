#version 330 core

struct NoiseLayer {
    float frequency;
    float amplitude;
    bool enabled;
};

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

in vec2 uv_frag_in;
in vec3 normal_frag_in;
in vec3 tangent_frag_in;
in vec3 bitangent_frag_in;
in vec3 model_position;

out vec4 color;

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform vec3 cameraPosition;

uniform bool showNormals;
uniform bool showTangents;
uniform bool showUVs;

const int MAX_NUM_NOISE_LAYERS = 15;
uniform NoiseLayer noiseLayers[MAX_NUM_NOISE_LAYERS];
uniform int numNoiseLayers;

uniform float grassLevel;
uniform float rockLevel;
uniform float blur;

uniform vec3 lightPosition;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform float lightPower;

//uniform sampler2D normalSampler;
uniform sampler2D grassTexture;

vec3 grassColor = vec3(19.0F/255.0F, 133.0F/255.0F, 16.0F/255.0F);
vec3 rockColor = vec3(73.0F/255.0F, 60.0F/255.0F, 60.0F/255.0F);
vec3 snowColor = vec3(255.0F/255.0F, 250.0F/255.0F, 250.0F/255.0F);

vec3 getSurfaceColor(float height) {
    vec3 grass = texture(grassTexture, uv_frag_in).rgb;
    return grass;

    float noiseMin = 0.0F;
    float noiseMax = 0.0F;
    // TODO this can be done in the TES instead to increase performance
    for (int i = 0; i < numNoiseLayers; i++) {
        if (!noiseLayers[i].enabled) {
            continue;
        }
        noiseMin -= noiseLayers[i].amplitude;
        noiseMax += noiseLayers[i].amplitude;
    }

    height /= noiseMax - noiseMin;

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

vec4 debugColors(vec3 normal, vec3 tangent, vec3 bitangent) {
    if (showUVs) {
        // this uv is only used for visualization (it shows the borders between neighboring textures)
        vec2 uv = fract(uv_frag_in);
        return vec4(uv, 0.0F, 1.0F);
    }

    if (showNormals) {
        normal += 1.0F;
        normal /= 2.0F;
        return vec4(normal, 1.0F);
    }

    if (showTangents) {
        tangent += 1.0F;
        tangent /= 2.0F;

        bitangent += 1.0F;
        bitangent /= 2.0F;

        return vec4(tangent.y, bitangent.y, 0.0F, 1.0F);
    }

    return vec4(1.0F, 1.0F, 1.0F, 1.0F);
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

vec3 calcPointLightComplex(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    return vec3(1.0F);
    /*
        vec3 lightDir = normalize(light.position - fragPos);
        // diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);
        // specular shading
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        // attenuation
        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        // combine results
        vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
        vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
        vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
        return (ambient + diffuse + specular);
    */
}

vec4 calcPointLightSimple(PointLight light, Material material, vec3 normal, vec3 viewDir) {
    vec3 lightDir = light.position - model_position;
    float distance = length(lightDir);

    lightDir = normalize(lightDir);
    float cosTheta = clamp(dot(normal, lightDir), 0, 1);
    vec3 reflectDir = reflect(-lightDir, normal);
    float cosAlpha = clamp(dot(viewDir, reflectDir), 0, 1);

    vec3 color = material.ambient;
    color += lightColor * lightPower * material.diffuse * cosTheta / (distance*distance);
    color += lightColor * lightPower * material.specular * pow(cosAlpha, material.shininess) / (distance*distance);
    return vec4(color, 1.0F);
}

void main() {
    vec3 normal = normalize(normalMatrix * normal_frag_in);
    vec3 tangent = normalize(normalMatrix * tangent_frag_in);
    vec3 bitangent = normalize(normalMatrix * bitangent_frag_in);
    vec3 viewDir = normalize(cameraPosition - model_position);

    if (showNormals || showTangents || showUVs) {
        color = debugColors(normal, tangent, bitangent);
        return;
    }

    vec3 materialDiffuseColor = getSurfaceColor(model_position.y);
    vec3 materialAmbientColor = vec3(0.1, 0.1, 0.1) * materialDiffuseColor;
    vec3 materialSpecularColor = vec3(0.3, 0.3, 0.3);
    Material material = Material(materialAmbientColor, materialDiffuseColor, materialSpecularColor, 2);
    PointLight light = PointLight(lightPosition, lightColor, lightPower);

    DirLight dirLight = DirLight(lightDirection, lightColor, lightPower);

    color = vec4(0.0F);
    color += calcPointLightSimple(light, material, normal, viewDir);
    color += calcDirLight(dirLight, material, normal, viewDir);
}

vec3 getNormalFromNormalMap() {
    #if 1
    return vec3(0.0, 1.0F, 0.0F);
    #else
    float overlap = 0.1F;

    vec2 uv = vUV;
    uv.x = fract(uv.x);
    uv.x *= 1.0F - overlap;
    uv.x += overlap * 0.5F;

    vec3 normal = texture(uNormalSampler, uv).rgb;

    float xFrac = fract(uv.x);
    float xFracInv = 1.0F - xFrac;
    if (xFrac > 1.0F - overlap) {
        vec2 uvNext = vec2(overlap - xFracInv, uv.y);
        vec3 normalNext = texture(uNormalSampler, uvNext).rgb;
        normal = mix(normal, normalNext, 1.0F - ((1.0F - xFrac) * (1.0F / overlap)));
    }
    if (xFrac < overlap) {
        vec2 uvNext = vec2(1.0F - (overlap - xFrac), uv.y);
        vec3 normalNext = texture(uNormalSampler, uvNext).rgb;
        normal = mix(normal, normalNext, 1.0F - (xFrac * (1.0F / overlap)));
    }

    float yFrac = fract(uv.y);
    float yFracInv = 1.0F - yFrac;
    if (yFrac > 1.0F - overlap) {
        vec2 uvNext = vec2(overlap - yFracInv, uv.y);
        vec3 normalNext = texture(uNormalSampler, uvNext).rgb;
        normal = mix(normal, normalNext, 1.0F - ((1.0F - yFrac) * (1.0F / overlap)));
    }
    if (yFrac < overlap) {
        vec2 uvNext = vec2(1.0F - (overlap - yFrac), uv.y);
        vec3 normalNext = texture(uNormalSampler, uvNext).rgb;
        normal = mix(normal, normalNext, 1.0F - (yFrac * (1.0F / overlap)));
    }

    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(vTBN * normal);

    return normal;
    #endif
}
