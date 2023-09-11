#version 300 es
precision mediump float;

in vec3 a_Position;
in vec2 a_UV;

uniform vec2 uvScale;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 UV;

void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_Position, 1.0F);

    vec2 finalUvScale = 1.0 + (1.0 - uvScale);
    vec2 result = a_UV;
    result -= 0.5;
    result *= finalUvScale;
    result.y *= -1.0;
    result += 0.5;
    UV = result;
}
