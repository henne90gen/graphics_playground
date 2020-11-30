#version 330

in float vHeight;
flat in int batchId;
in vec3 vPosition;
in vec3 normal;

uniform float waterLevel;
uniform float grassLevel;
uniform float rockLevel;
uniform float blur;

uniform vec3 surfaceToLight;
uniform vec3 lightColor;
uniform float lightPower;

uniform bool showBatchIds;

out vec4 color;

vec4 waterColor = vec4(28.0F/255.0F, 163.0F/255.0F, 236.0F/255.0F, 1.0F);
vec4 grassColor = vec4(19.0F/255.0F, 133.0F/255.0F, 16.0F/255.0F, 1.0F);
vec4 rockColor = vec4(73.0F/255.0F, 60.0F/255.0F, 60.0F/255.0F, 1.0F);
vec4 snowColor = vec4(255.0F/255.0F, 250.0F/255.0F, 250.0F/255.0F, 1.0F);

vec3 batchColors[] = vec3[3](vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1));

void main() {
    float height = vHeight;
    if (height < waterLevel) {
        color = waterColor;
    } else if (height < grassLevel-blur) {
        color = grassColor;
    } else if (height < grassLevel+blur) {
        float t = (height-(grassLevel-blur)) / (2.0F*blur);
        color = mix(grassColor, rockColor, t);
    } else if (height < rockLevel-blur){
        color = rockColor;
    } else if (height < rockLevel+blur) {
        float t = (height-(rockLevel-blur)) / (2.0F*blur);
        color = mix(rockColor, snowColor, t);
    } else {
        color = snowColor;
    }


    vec3 position = vPosition;
    vec3 normal_ = normalize(normal);

    float brightness = dot(normal_, surfaceToLight);
    brightness *= lightPower;
    brightness = clamp(brightness, 0, 1);

    vec3 diffuseColor = brightness * lightColor * color.rgb;

    vec3 batchColor = batchColors[batchId % batchColors.length()];
    vec3 colorv3 = float(!showBatchIds) * diffuseColor + batchColor * float(showBatchIds);
    color = vec4(colorv3, 1.0F);
}
