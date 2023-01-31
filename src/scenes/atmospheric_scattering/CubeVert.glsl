#version 300 es
precision mediump float;

#include "ScatterLib.glsl"

in vec3 a_Position;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 cameraPosition;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform float lightPower;

out vec3 modelPosition;
out vec3 extinction;
out vec3 inScatter;

void main() {
    vec4 position = modelMatrix * vec4(a_Position, 1.0F);
    modelPosition = position.xyz;
    gl_Position = projectionMatrix * viewMatrix * position;

    calcScattering(cameraPosition, modelPosition, lightDirection, lightColor, lightPower, extinction, inScatter);
}
