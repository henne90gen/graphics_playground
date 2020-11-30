#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_UV;

out vec2 UV;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix = mat4(1.0);
uniform mat4 u_ProjectionMatrix = mat4(1.0);

void main() {
    UV = a_UV;
    gl_Position = u_ProjectionMatrix * u_ViewMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
}
