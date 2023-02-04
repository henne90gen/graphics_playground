#version 300 es
precision mediump float;

struct Light {
    vec3 position;
    vec3 color;
};

in vec3 v_Position;
in vec2 v_UV;
in vec3 v_Normal;
in mat3 v_TBN;
in vec3 v_CameraPosition;

uniform mat3 u_NormalMatrix;
uniform Light u_Light;
uniform sampler2D u_TextureSampler;
uniform sampler2D u_NormalSampler;
uniform bool u_UseNormalMap;

out vec4 color;

void main() {
    vec3 normal = vec3(1.0F);
    if (u_UseNormalMap) {
        normal = texture(u_NormalSampler, v_UV).rgb;
        normal = normalize(normal * 2.0F - 1.0F);
        normal = normalize(v_TBN * normal);
    } else {
        normal = normalize(u_NormalMatrix * v_Normal);
    }

    vec3 surfaceToLight = u_Light.position - v_Position;
    float distanceToLight = length(surfaceToLight);

    float brightness = dot(normal, surfaceToLight) / (distanceToLight * distanceToLight);
    brightness = clamp(brightness, 0.0F, 1.0F);

    vec4 surfaceColor = texture(u_TextureSampler, v_UV);
    vec3 diffuseColor = brightness * u_Light.color * surfaceColor.rgb;

    vec3 specularColor = vec3(0.1F);
    vec3 cameraDirection = normalize(v_CameraPosition - v_Position);
    vec3 reflectionDirection = reflect(surfaceToLight, normal);
    float cosAlpha = clamp(dot(cameraDirection, reflectionDirection), 0.0F, 1.0F);
    specularColor *= u_Light.color * pow(cosAlpha, 5.0F) / (distanceToLight * distanceToLight);

    vec3 colorv3 = vec3(0.0F);
    vec3 ambientColor = vec3(0.1F);
    colorv3 += ambientColor;
    colorv3 += diffuseColor;
    colorv3 += specularColor;
    color = vec4(colorv3, surfaceColor.a);
}
