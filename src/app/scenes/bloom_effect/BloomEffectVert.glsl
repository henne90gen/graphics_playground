#version 130

attribute vec3 a_Position;
attribute vec3 a_Normal;
attribute vec2 a_UV;

uniform mat4 u_Model;
uniform mat3 u_NormalMatrix;
uniform mat4 u_View;
uniform mat4 u_Projection;

varying vec3 v_Position;
varying vec3 v_Normal;
varying vec2 v_UV;
varying vec3 v_CameraPosition;

void main() {
    vec4 worldPosition = u_Model * vec4(a_Position, 1);
    v_Position = vec3(worldPosition);
    v_Normal = normalize(u_NormalMatrix * a_Normal);
    v_UV = a_UV;
    v_CameraPosition = vec3(u_View * vec4(0.0, 0.0, 0.0, 1.0));
    gl_Position = u_Projection * u_View * worldPosition;
}
