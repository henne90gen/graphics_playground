#version 410

in vec3 v_Position;
in vec3 v_Normal;
in vec3 v_CameraPosition;

uniform vec3 u_LightPosition;
uniform vec3 u_LightDirection;
uniform vec3 u_LightColor;

uniform vec3 u_AmbientColor;
uniform vec3 u_SpecularColor;
uniform bool u_UseAmbient;
uniform bool u_UseDiffuse;
uniform bool u_UseSpecular;
uniform int u_FalloffType;
uniform float u_FalloffPosition;
uniform float u_FalloffSpeed;

out vec4 o_Color;

#define PI 3.1415926535897932384626433832795

void main() {
    vec3 surfaceToLight = u_LightPosition - v_Position;
    float distanceToLight = length(surfaceToLight);
    float distanceToLightSqr = distanceToLight * distanceToLight;
    vec3 cameraDirection = normalize(v_CameraPosition - v_Position);

    float brightness = dot(v_Normal, surfaceToLight) / distanceToLightSqr;
    brightness = clamp(brightness, 0, 1);

    vec4 surfaceColor = vec4(1.0, 1.0, 1.0, 1.0);
    float cosTheta = dot(-1 * normalize(surfaceToLight), normalize(u_LightDirection));
    float theta = acos(cosTheta);
    float falloff;
    if (u_FalloffType == 0) {
        // cosine falloff
        float piHalf = PI / 2.0F;
        float widthHalf = piHalf * (1/u_FalloffSpeed);
        if (theta <= u_FalloffPosition - widthHalf) {
            falloff = 1;
        } else if (theta >= u_FalloffPosition + widthHalf) {
            falloff = 0;
        } else {
            falloff = 0.5F * (1 + cos(u_FalloffSpeed * (theta - u_FalloffPosition) + piHalf));
        }
    } else if (u_FalloffType == 1) {
        // sigmoid falloff
        falloff = 1 - 1 / (1 + exp(-1.0F * (u_FalloffSpeed * u_FalloffSpeed * (theta - u_FalloffPosition))));
    } else if (u_FalloffType == 2) {
        // linear falloff
        if (theta <= -1.0F / u_FalloffSpeed + u_FalloffPosition) {
            falloff = 1;
        } else if (theta >= 1.0F / u_FalloffSpeed + u_FalloffPosition) {
            falloff = 0;
        } else {
            falloff = 0.5F + -0.5F * u_FalloffSpeed * (theta - u_FalloffPosition);
        }
    }
    vec3 diffuseColor = brightness * u_LightColor * surfaceColor.rgb * falloff;

    vec3 reflectionDirection = reflect(surfaceToLight, v_Normal);
    float cosAlpha = clamp(dot(cameraDirection, reflectionDirection), 0, 1);
    vec3 specularColor = (u_SpecularColor * u_LightColor * pow(cosAlpha, 5)) / distanceToLightSqr;

    vec3 color = vec3(0.0);
    if (u_UseAmbient) {
        color += u_AmbientColor;
    }
    if (u_UseDiffuse) {
        color += diffuseColor;
    }
    if (u_UseSpecular) {
        color += specularColor;
    }
    o_Color = vec4(color, surfaceColor.a);
}
