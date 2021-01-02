#version 330 core

in vec3 a_Position;

out vec3 position_frag_in;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_Position, 1.0F);
    position_frag_in = a_Position;
}
