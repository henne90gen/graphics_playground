#version 300 es
precision mediump float;

in vec3 extinction;
in vec3 inScatter;

uniform bool showFex;
uniform bool useFex;
uniform bool showLin;
uniform bool useLin;

out vec4 color;

void main() {
    color = vec4(0.529F, 0.808F, 0.922F, 1.0F);

    if (useFex) {
        color *= vec4(extinction, 1.0F);
    }
    if (showFex) {
        color = vec4(extinction, 1.0F);
    }

    if (useLin) {
        color += vec4(inScatter, 0.0F);
    }
    if (showLin) {
        color = vec4(inScatter, 1.0F);
    }
}
