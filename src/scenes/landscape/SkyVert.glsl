#version 330 core

in vec3 a_Position;
in vec3 a_Normal;

out vec3 position_frag_in;
out vec3 normal_frag_in;
out vec3 model_position_frag_in;

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    vec4 position = viewMatrix * modelMatrix * vec4(a_Position, 1.0F);
    position_frag_in = position.xyz;
    model_position_frag_in = a_Position;
    normal_frag_in = normalMatrix * a_Normal;
    gl_Position = projectionMatrix * position;
}
