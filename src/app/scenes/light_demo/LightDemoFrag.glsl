#version 130

struct Light {
    vec3 position;
    vec3 color;
};

varying vec3 v_Position;
varying vec3 v_Normal;
varying vec2 v_UV;

uniform sampler2D u_TextureSampler;

uniform Light u_Light;

void main() {
    vec3 surfaceToLight = u_Light.position - v_Position;

    float brightness = dot(v_Normal, surfaceToLight) / (length(surfaceToLight) * length(v_Normal));
    brightness = clamp(brightness, 0, 1);

    vec4 surfaceColor = texture(u_TextureSampler, v_UV);
    gl_FragColor = vec4(brightness * u_Light.color * surfaceColor.rgb, surfaceColor.a);
}
