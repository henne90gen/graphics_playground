#version 330 core

in vec3 Fex;
in vec3 Lin;

uniform bool showFex;
uniform bool useFex;
uniform bool showLin;
uniform bool useLin;

out vec4 color;

void main() {
    color = vec4(0.529F, 0.808F, 0.922F, 1.0F);

    if (useFex) {
        color *= vec4(Fex, 1.0F);
    }
    if (showFex) {
        color = vec4(Fex, 1.0F);
    }

    if (useLin) {
        color += vec4(Lin, 0.0F);
    }
    if (showLin) {
        color = vec4(Lin, 1.0F);
    }
}
