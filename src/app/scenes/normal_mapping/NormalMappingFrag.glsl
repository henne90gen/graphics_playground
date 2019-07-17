#version 130

struct Light {
    vec3 position;
    vec3 color;
};

varying vec3 v_Position;
varying vec2 v_UV;
varying vec3 v_Normal;
varying mat3 v_TBN;
varying vec3 v_CameraPosition;

uniform mat3 u_NormalMatrix;
uniform Light u_Light;
uniform sampler2D u_TextureSampler;
uniform sampler2D u_NormalSampler;
uniform bool u_UseNormalMap;

void main() {
    vec3 normal = vec3(1.0);
    if (u_UseNormalMap) {
        normal = texture(u_NormalSampler, v_UV).rgb;
        normal = normalize(normal * 2.0 - 1.0);
        normal = normalize(v_TBN * normal);
    } else {
        normal = normalize(u_NormalMatrix * v_Normal);
    }

    vec3 surfaceToLight = u_Light.position - v_Position;
    float distanceToLight = length(surfaceToLight);

    float brightness = dot(normal, surfaceToLight) / (distanceToLight * distanceToLight);
    brightness = clamp(brightness, 0, 1);

    vec4 surfaceColor = texture(u_TextureSampler, v_UV);
    vec3 diffuseColor = brightness * u_Light.color * surfaceColor.rgb;

    vec3 cameraDirection = normalize(v_CameraPosition - v_Position);
    vec3 reflectionDirection = reflect(surfaceToLight, normal);
    float cosAlpha = clamp(dot(cameraDirection, reflectionDirection), 0, 1);

    vec3 specularColor = vec3(0.1);
    specularColor *= u_Light.color * pow(cosAlpha, 5) / (distanceToLight * distanceToLight);

    vec3 color = vec3(0.0);
    vec3 ambientColor = vec3(0.2);
    color += ambientColor;
    color += diffuseColor;
    color += specularColor;
    gl_FragColor = vec4(color, surfaceColor.a);
}
