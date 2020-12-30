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
uniform vec3 lightColor;
uniform float lightPower;

uniform sampler2D normalSampler;

vec3 grassColor = vec3(19.0F/255.0F, 133.0F/255.0F, 16.0F/255.0F);
vec3 rockColor = vec3(73.0F/255.0F, 60.0F/255.0F, 60.0F/255.0F);
vec3 snowColor = vec3(255.0F/255.0F, 250.0F/255.0F, 250.0F/255.0F);

vec3 getSurfaceColor(float height) {
    return rockColor;

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
    if (showNormals) {
        //        normal = normal_frag_in;
        normal += 1.0F;
        normal /= 2.0F;
        return vec4(normal, 1.0F);
    }

    if (showTangents) {
        //        tangent = tangent_frag_in;
        tangent += 1.0F;
        tangent /= 2.0F;

        //        bitangent = bitangent_frag_in;
        bitangent += 1.0F;
        bitangent /= 2.0F;

        return vec4(tangent.y, bitangent.y, 0.0F, 1.0F);
    }

    if (showUVs) {
        // this uv is only used for visualization (it shows the borders between neighboring textures)
        vec2 uv = fract(uv_frag_in + 500);
        return vec4(uv.x, uv.y, 0.0F, 1.0F);
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
    // combine results
    vec3 lightColor = light.color * light.power;
    vec3 ambient  = lightColor * material.ambient;
    vec3 diffuse  = lightColor * diff * material.diffuse;
    vec3 specular = lightColor * spec * material.specular;
    vec3 result = ambient + diffuse + specular;
    return vec4(result, 1.0F);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    return vec3(1.0F);
    #if 0
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
    #endif
}

vec4 calcLight3(PointLight light, Material material, vec3 normal, vec3 viewDir) {
    vec3 lightDir = light.position - model_position;
    float distance = length(lightDir);

    lightDir = normalize(lightDir);
    // Cosine of the angle between the normal and the light direction,
    // clamped above 0
    //  - light is at the vertical of the triangle -> 1
    //  - light is perpendicular to the triangle -> 0
    //  - light is behind the triangle -> 0
    float cosTheta = clamp(dot(normal, lightDir), 0, 1);

    // Direction in which the triangle reflects the light
    vec3 reflectDir = reflect(-lightDir, normal);
    // Cosine of the angle between the Eye vector and the Reflect vector,
    // clamped to 0
    //  - Looking into the reflection -> 1
    //  - Looking elsewhere -> < 1
    float cosAlpha = clamp(dot(viewDir, reflectDir), 0, 1);

    vec3 color = material.ambient;
    color += material.diffuse * lightColor * lightPower * cosTheta / (distance*distance);
    color += material.specular * lightColor * lightPower * pow(cosAlpha, material.shininess) / (distance*distance);
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
    Material material = Material(materialAmbientColor, materialDiffuseColor, materialSpecularColor, 8);
    PointLight light = PointLight(lightPosition, lightColor, lightPower);
    DirLight dirLight = DirLight(lightPosition, lightColor, lightPower);

    color = vec4(0.0F);
    color += calcLight3(light, material, normal, viewDir);
//        color += calcDirLight(dirLight, material, normal, viewDir);
}

vec4 calcLight2() {
    return vec4(1.0F, 1.0F, 1.0F, 1.0F);
    #if 0
    vec3 materialAmbient = vec3(0.1F);
    vec3 materialDiffuse = getSurfaceColor(model_position.y);
    vec3 materialSpecular = vec3(0.1F);
    float materialShininess = 8;

    // ambient
    vec3 ambient = lightColor * materialAmbient;

    // diffuse
    //vec3 lightDir = normalize(lightPosition - model_position);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = lightColor * (diff * materialDiffuse);

    // specular
    vec3 viewDir = normalize(cameraPosition - model_position);
    //    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(dot(viewDir, reflectDir), materialShininess);
    vec3 specular = lightColor * (spec * materialSpecular);

    vec3 result = vec3(0.0F);
    result += ambient;
    result += diffuse;
    //    result += specular;
    return vec4(result, 1.0F);
    #endif
}

vec4 calcLight1() {
    return vec4(1.0F, 1.0F, 1.0F, 1.0F);
    #if 0
    float brightness = max(dot(normal, surfaceToLight), 0.0F);
    brightness *= lightPower;
    brightness = clamp(brightness, 0, 1);

    vec3 surfaceColor = getSurfaceColor(model_position.y);
    vec3 diffuseColor = brightness * lightColor * surfaceColor;

    vec3 specularColor = vec3(0.001F);
    vec3 cameraDirection = normalize(cameraPosition - model_position);
    vec3 reflectionDirection = reflect(surfaceToLight, normal);
    float cosAlpha = max(dot(cameraDirection, reflectionDirection), 0.0F);
    specularColor *= lightColor * pow(cosAlpha, 2) * lightPower;

    vec3 color = vec3(0.0);
    vec3 ambientColor = vec3(0.1F);
    color += ambientColor;
    color += diffuseColor;
    color += specularColor;

    return vec4(color, 1.0F);
    #endif
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
