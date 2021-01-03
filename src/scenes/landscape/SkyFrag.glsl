#version 330 core

in vec3 position_frag_in;

uniform vec3 flatColor;
uniform float animationTime;
uniform float animationSpeed;
uniform float cloudBlend;

out vec4 color;

const float cloudscale = 1.1;
const float clouddark = 0.5;
const float cloudlight = 0.3;
const float cloudcover = 0.2;
const float cloudalpha = 8.0;
const float skytint = 0.5;
const vec3 skycolour1 = vec3(0.2, 0.4, 0.6);
const vec3 skycolour2 = vec3(0.4, 0.7, 1.0);

const mat2 m = mat2(1.6, 1.2, -1.2, 1.6);

vec2 hash(vec2 p) {
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

float noise(in vec2 p) {
    const float K1 = 0.366025404;// (sqrt(3)-1)/2;
    const float K2 = 0.211324865;// (3-sqrt(3))/6;
    vec2 i = floor(p + (p.x+p.y)*K1);
    vec2 a = p - i + (i.x+i.y)*K2;
    vec2 o = (a.x>a.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);//vec2 of = 0.5 + 0.5*vec2(sign(a.x-a.y), sign(a.y-a.x));
    vec2 b = a - o + K2;
    vec2 c = a - 1.0 + 2.0*K2;
    vec3 h = max(0.5-vec3(dot(a, a), dot(b, b), dot(c, c)), 0.0);
    vec3 n = h*h*h*h*vec3(dot(a, hash(i+0.0)), dot(b, hash(i+o)), dot(c, hash(i+1.0)));
    return dot(n, vec3(70.0));
}

float fbm(vec2 n) {
    float total = 0.0, amplitude = 0.1;
    for (int i = 0; i < 7; i++) {
        total += noise(n) * amplitude;
        n = m * n;
        amplitude *= 0.4;
    }
    return total;
}

// https://www.shadertoy.com/view/4tdSWr
vec4 cloudColor(vec3 skycolor, float iTime) {
    vec2 uv_ = position_frag_in.xz + 0.5F;
    vec2 uv = uv_;
    float time = iTime * animationSpeed;
    float q = fbm(uv * cloudscale * 0.5);

    // ridged noise shape
    float r = 0.0;
    uv *= cloudscale;
    uv -= q - time;
    float weight = 0.8;
    for (int i=0; i<8; i++){
        r += abs(weight*noise(uv));
        uv = m*uv + time;
        weight *= 0.7;
    }

    // noise shape
    float f = 0.0;
    uv = uv_;
    uv *= cloudscale;
    uv -= q - time;
    weight = 0.7;
    for (int i=0; i<8; i++){
        f += weight*noise(uv);
        uv = m*uv + time;
        weight *= 0.6;
    }

    f *= r + f;

    // noise color
    float c = 0.0;
    time = iTime * animationSpeed * 2.0;
    uv = uv_;
    uv *= cloudscale*2.0;
    uv -= q - time;
    weight = 0.4;
    for (int i=0; i<7; i++){
        c += weight*noise(uv);
        uv = m*uv + time;
        weight *= 0.6;
    }

    // noise ridge colour
    float c1 = 0.0;
    time = iTime * animationSpeed * 3.0;
    uv = uv_;
    uv *= cloudscale*3.0;
    uv -= q - time;
    weight = 0.4;
    for (int i=0; i<7; i++){
        c1 += abs(weight*noise(uv));
        uv = m*uv + time;
        weight *= 0.6;
    }

    c += c1;

    vec3 cloudcolor = vec3(1.1, 1.1, 0.9) * clamp((clouddark + cloudlight*c), 0.0, 1.0);

    f = cloudcover + cloudalpha*f*r;

    vec3 result = mix(skycolor, clamp(skytint * skycolor + cloudcolor, 0.0, 1.0), clamp(f + c, 0.0, 1.0));

    uv = uv_;
    float oneMinusB = 1.0F - cloudBlend;
    float bInv = 1.0F / cloudBlend;
    float tRight = 1.0F - clamp((uv.x - oneMinusB) * bInv, 0.0F, 1.0F);
    float tLeft = clamp(uv.x * bInv, 0.0F, 1.0F);
    float tTop = 1.0F - clamp((uv.y - oneMinusB) * bInv, 0.0F, 1.0F);
    float tBottom = clamp(uv.y * bInv, 0.0F, 1.0F);
    float t = tRight * tLeft * tTop * tBottom;
    result = mix(skycolor, result, t);

    return vec4(result, 1.0);
}

void main() {
    float height = position_frag_in.y;
    color = vec4(flatColor * (-height + 1.4F), 1.0F);
    if (height >= 0.4999F) {
        color = cloudColor(color.xyz, animationTime);
    }
}
