#version 130

struct Light {
    vec3 position;
    vec3 color;
};

varying vec3 v_Position;
varying vec3 v_Normal;
varying vec2 v_UV;
varying vec3 v_CameraPosition;

uniform sampler2D u_TextureSampler;
uniform vec3 u_AmbientColor;
uniform vec3 u_SpecularColor;
uniform Light u_Light;

void main() {
    vec3 surfaceToLight = u_Light.position - v_Position;
    float distanceToLight = length(surfaceToLight);

    float brightness = dot(v_Normal, surfaceToLight) / (distanceToLight * distanceToLight);
    brightness = clamp(brightness, 0, 1);

    vec4 surfaceColor = texture(u_TextureSampler, v_UV);
    vec3 diffuseColor = brightness * u_Light.color * surfaceColor.rgb;

    vec3 cameraDirection = normalize(v_CameraPosition - v_Position);
    vec3 reflectionDirection = reflect(surfaceToLight, v_Normal);
    float cosAlpha = clamp(dot(cameraDirection, reflectionDirection), 0, 1);

    vec3 specularColor = u_SpecularColor * u_Light.color * pow(cosAlpha, 5) / (distanceToLight * distanceToLight);

    vec3 color = vec3(0.0);
    color += u_AmbientColor;
    color += diffuseColor;
    color += specularColor;
    gl_FragColor = vec4(color, surfaceColor.a);
}
