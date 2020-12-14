#version 330

in float vHeight;
in vec3 vPosition;
in vec2 vUV;
in vec3 vNormal;
in mat3 vTBN;
in vec3 vCameraPosition;

uniform float waterLevel;
uniform float grassLevel;
uniform float rockLevel;
uniform float blur;

uniform vec3 surfaceToLight;
uniform vec3 lightColor;
uniform float uLightPower;
uniform sampler2D uNormalSampler;
uniform bool useNormalMap;
uniform bool showUVs;

out vec4 color;

vec4 grassColor = vec4(19.0F/255.0F, 133.0F/255.0F, 16.0F/255.0F, 1.0F);
vec4 rockColor = vec4(73.0F/255.0F, 60.0F/255.0F, 60.0F/255.0F, 1.0F);
vec4 snowColor = vec4(255.0F/255.0F, 250.0F/255.0F, 250.0F/255.0F, 1.0F);

vec4 getSurfaceColor(float height) {
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

vec3 getNormalFromNormalMap() {
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
}

void main() {
    if (showUVs) {
        color = vec4(vUV.r, vUV.g, 0.0F, 1.0F);
        return;
    }

    float lightPower = uLightPower;

    vec3 normal = vec3(1.0);
    if (useNormalMap) {
        normal = getNormalFromNormalMap();
    } else {
        normal = normalize(vNormal);
        lightPower += 200.0F;
    }

    float distanceToLight = length(surfaceToLight);
    float brightness = dot(normal, surfaceToLight) / (distanceToLight * distanceToLight);
    brightness *= lightPower;
    brightness = clamp(brightness, 0, 1);

    vec4 surfaceColor = getSurfaceColor(vHeight);
    vec3 diffuseColor = brightness * lightColor * surfaceColor.rgb;

    vec3 specularColor = vec3(0.1);
    vec3 cameraDirection = normalize(vCameraPosition - vPosition);
    vec3 reflectionDirection = reflect(surfaceToLight, normal);
    float cosAlpha = clamp(dot(cameraDirection, reflectionDirection), 0, 1);
    specularColor *= lightColor * pow(cosAlpha, 5) / (distanceToLight * distanceToLight);

    vec3 colorv3 = vec3(0.0);
    vec3 ambientColor = vec3(0.1);
    colorv3 += ambientColor;
    colorv3 += diffuseColor;
    colorv3 += specularColor;

    color = vec4(colorv3, 1.0F);
}
