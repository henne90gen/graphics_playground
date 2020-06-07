#version 130

attribute vec3 a_Position;
attribute vec3 a_Normal;

uniform mat4 u_ModelMatrix;
uniform mat3 u_NormalMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

varying vec3 v_Position;
varying vec3 v_Normal;
varying vec3 v_CameraPosition;

void main() {
    vec4 worldPosition = u_ModelMatrix * vec4(a_Position, 1);
    v_Position = vec3(worldPosition);
    v_Normal = normalize(u_NormalMatrix * a_Normal);
    v_CameraPosition = vec3(u_ViewMatrix * vec4(0.0, 0.0, 0.0, 1.0));
    gl_Position = u_ProjectionMatrix * u_ViewMatrix * worldPosition;
}
