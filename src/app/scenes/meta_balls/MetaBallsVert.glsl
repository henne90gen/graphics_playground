#version 330 core

in vec3 position;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec3 u_Offset;

out vec3 v_FinalPosition;

void main() {
    v_FinalPosition = position + u_Offset;
    gl_Position = u_Projection * u_View * u_Model * vec4(v_FinalPosition, 1.0);
}
