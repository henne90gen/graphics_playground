#version 300 es
precision mediump float;

struct Light {
    vec3 position;
    vec3 color;
};

in vec3 v_Position;
in vec3 v_Normal;
in vec2 v_UV;
in vec3 v_CameraPosition;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform sampler2D u_TextureSampler;
uniform vec3 u_AmbientColor;
uniform vec3 u_SpecularColor;
uniform Light u_Light;
uniform float u_Threshold;

void main() {
    vec3 surfaceToLight = u_Light.position - v_Position;
    float distanceToLight = length(surfaceToLight);

    float brightness = dot(v_Normal, surfaceToLight) / (distanceToLight * distanceToLight);
    brightness = clamp(brightness, 0.0F, 1.0F);

    vec4 surfaceColor = texture(u_TextureSampler, v_UV);
    vec3 diffuseColor = brightness * u_Light.color * surfaceColor.rgb;

    vec3 cameraDirection = normalize(v_CameraPosition - v_Position);
    vec3 reflectionDirection = reflect(surfaceToLight, v_Normal);
    float cosAlpha = clamp(dot(cameraDirection, reflectionDirection), 0.0F, 1.0F);

    vec3 specularColor = u_SpecularColor * u_Light.color * pow(cosAlpha, 5.0F) / (distanceToLight * distanceToLight);

    vec3 color = vec3(0.0);
    color += u_AmbientColor;
    color += diffuseColor;
    color += specularColor;
    FragColor = vec4(color, surfaceColor.a);

    // check whether fragment output is higher than threshold, if so output as brightness color
    brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > u_Threshold) {
        BrightColor = vec4(FragColor.rgb, 1.0);
    } else {
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
