#version 330 core

in vec3 a_Position;
in vec3 a_Normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 normal_frag_in;
out vec3 model_position;

void main() {
    vec4 position = viewMatrix * modelMatrix * vec4(a_Position, 1.0F);
    model_position = position.xyz;
    gl_Position = projectionMatrix * position;
    normal_frag_in = a_Normal;
}
