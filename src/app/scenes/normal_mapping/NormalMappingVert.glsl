#version 130

attribute vec3 a_Position;
attribute vec2 a_UV;
attribute vec3 a_Normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

varying vec3 v_Position;
varying vec2 v_UV;
varying vec3 v_Normal;
varying vec3 v_CameraPosition;

void main() {
    //    vec3 T = normalize(vec3(u_Model * vec4(a_Tangent, 0.0)));
    //    vec3 B = normalize(vec3(u_Model * vec4(a_BiTangent, 0.0)));
    //    vec3 N = normalize(vec3(u_Model * vec4(a_Normal, 0.0)));
    //    mat3 TBN = mat3(T, B, N);

    vec4 worldPosition = u_Model * vec4(a_Position, 1.0);
    v_Position = vec3(worldPosition);

    v_Normal = a_Normal;
    v_UV = a_UV;
    v_CameraPosition = vec3(u_View * vec4(0.0, 0.0, 0.0, 1.0));
    gl_Position = u_Projection * u_View * worldPosition;
}
