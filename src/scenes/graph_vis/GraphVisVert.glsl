#version 300 es
precision mediump float;

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 translation;
layout (location = 2) in vec3 color;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform bool translationEnabled;
uniform bool useGlobalColor;
uniform vec3 uColor;

out vec3 oColor;

void main() {
    oColor = float(!useGlobalColor) * color + float(useGlobalColor) * uColor;
    gl_Position = projectionMatrix * viewMatrix * vec4(position + translation * float(translationEnabled), 0.0F, 1.0F);
}
