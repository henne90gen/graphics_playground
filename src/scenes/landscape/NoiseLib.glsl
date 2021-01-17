const float PHI = 1.61803398874989484820459;// = Golden Ratio
float gold_noise(in vec2 uv, in float seed) {
    return fract(tan(distance(uv * PHI, uv) * seed) * uv.x);
}
