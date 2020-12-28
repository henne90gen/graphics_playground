#version 440 core

struct NoiseLayer {
    float frequency;
    float amplitude;
};

layout (triangles, equal_spacing, cw) in;

in vec2 position_tes_in[];
in vec3 ev_tes_in[];

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform float uvScaleFactor;

const int MAX_NUM_NOISE_LAYERS = 15;
uniform NoiseLayer noiseLayers[MAX_NUM_NOISE_LAYERS];
uniform int numNoiseLayers;

out vec2 uv_frag_in;
out vec3 normal_frag_in;
out vec3 tangent_frag_in;
out vec3 bitangent_frag_in;
out vec3 camera_position;
out vec3 model_position;

// FAST32_hash
// A very fast hashing function.  Requires 32bit support.
// http://briansharpe.wordpress.com/2011/11/15/a-fast-and-simple-32bit-floating-point-hash-function/
//
// The 2D hash formula takes the form....
// hash = mod( coord.x * coord.x * coord.y * coord.y, SOMELARGEFLOAT ) / SOMELARGEFLOAT
// We truncate and offset the domain to the most interesting part of the noise.
// SOMELARGEFLOAT should be in the range of 400.0->1000.0 and needs to be hand picked.  Only some give good results.
// A 3D hash is achieved by offsetting the SOMELARGEFLOAT value by the Z coordinate
// https://github.com/BrianSharpe/GPU-Noise-Lib/blob/master/gpu_noise_lib.glsl
void FAST32_hash_2D(vec2 gridcell, out vec4 hash_0, out vec4 hash_1) { // generates 2 random numbers for each of the 4 cell corners
    // gridcell is assumed to be an integer coordinate
    const vec2 OFFSET = vec2(26.0, 161.0);
    const float DOMAIN = 71.0;
    const vec2 SOMELARGEFLOATS = vec2(951.135664, 642.949883);
    vec4 P = vec4(gridcell.xy, gridcell.xy + 1.0);
    P = P - floor(P * (1.0 / DOMAIN)) * DOMAIN;
    P += OFFSET.xyxy;
    P *= P;
    P = P.xzxz * P.yyww;
    hash_0 = fract(P * (1.0 / SOMELARGEFLOATS.x));
    hash_1 = fract(P * (1.0 / SOMELARGEFLOATS.y));
}

// SimplexPerlin2D_Deriv
// SimplexPerlin2D noise with derivatives
// returns vec3( value, xderiv, yderiv )
// https://github.com/BrianSharpe/GPU-Noise-Lib/blob/master/gpu_noise_lib.glsl
vec3 snoise2_deriv(vec2 P) {
    //	simplex math constants
    const float SKEWFACTOR = 0.36602540378443864676372317075294;// 0.5*(sqrt(3.0)-1.0)
    const float UNSKEWFACTOR = 0.21132486540518711774542560974902;// (3.0-sqrt(3.0))/6.0
    const float SIMPLEX_TRI_HEIGHT = 0.70710678118654752440084436210485;// sqrt( 0.5 )	height of simplex triangle
    const vec3 SIMPLEX_POINTS = vec3(1.0-UNSKEWFACTOR, -UNSKEWFACTOR, 1.0-2.0*UNSKEWFACTOR);//	vertex info for simplex triangle

    //	establish our grid cell.
    P *= SIMPLEX_TRI_HEIGHT;// scale space so we can have an approx feature size of 1.0  ( optional )
    vec2 Pi = floor(P + dot(P, vec2(SKEWFACTOR)));

    //	calculate the hash.
    //	( various hashing methods listed in order of speed )
    vec4 hash_x, hash_y;
    FAST32_hash_2D(Pi, hash_x, hash_y);
    //SGPP_hash_2D( Pi, hash_x, hash_y );

    //	establish vectors to the 3 corners of our simplex triangle
    vec2 v0 = Pi - dot(Pi, vec2(UNSKEWFACTOR)) - P;
    vec4 v1pos_v1hash = (v0.x < v0.y) ? vec4(SIMPLEX_POINTS.xy, hash_x.y, hash_y.y) : vec4(SIMPLEX_POINTS.yx, hash_x.z, hash_y.z);
    vec4 v12 = vec4(v1pos_v1hash.xy, SIMPLEX_POINTS.zz) + v0.xyxy;

    //	calculate the dotproduct of our 3 corner vectors with 3 random normalized vectors
    vec3 grad_x = vec3(hash_x.x, v1pos_v1hash.z, hash_x.w) - 0.49999;
    vec3 grad_y = vec3(hash_y.x, v1pos_v1hash.w, hash_y.w) - 0.49999;
    vec3 norm = inversesqrt(grad_x * grad_x + grad_y * grad_y);
    grad_x *= norm;
    grad_y *= norm;
    vec3 grad_results = grad_x * vec3(v0.x, v12.xz) + grad_y * vec3(v0.y, v12.yw);

    //	evaluate the surflet
    vec3 m = vec3(v0.x, v12.xz) * vec3(v0.x, v12.xz) + vec3(v0.y, v12.yw) * vec3(v0.y, v12.yw);
    m = max(0.5 - m, 0.0);//	The 0.5 here is SIMPLEX_TRI_HEIGHT^2
    vec3 m2 = m*m;
    vec3 m4 = m2*m2;

    //	calc the deriv
    vec3 temp = 8.0 * m2 * m * grad_results;
    float xderiv = dot(temp, vec3(v0.x, v12.xz)) - dot(m4, grad_x);
    float yderiv = dot(temp, vec3(v0.y, v12.yw)) - dot(m4, grad_y);

    const float FINAL_NORMALIZATION = 99.204334582718712976990005025589;//	scales the final result to a strict 1.0->-1.0 range

    //	sum the surflets and return all results combined in a vec3
    return vec3(dot(m4, grad_results), xderiv, yderiv) * FINAL_NORMALIZATION;
}

void main() {
    vec2 pos = gl_TessCoord.x * position_tes_in[0];
    pos     += gl_TessCoord.y * position_tes_in[1];
    pos     += gl_TessCoord.z * position_tes_in[2];

    vec3 noise = vec3(0.0F);
    for (int i = 0; i < numNoiseLayers; i++) {
        noise += snoise2_deriv(pos * (1.0F / noiseLayers[i].frequency)) * noiseLayers[i].amplitude;
    }
    float noiseValue = noise.x;
    tangent_frag_in = vec3(1.0F, noise.y, 0.0F);
    bitangent_frag_in = vec3(0.0F, noise.z, 1.0F);
    normal_frag_in = normalize(cross(tangent_frag_in, bitangent_frag_in));

    vec4 position = modelMatrix * vec4(pos.x, noiseValue, pos.y, 1.0F);
    model_position = position.xyz;
    gl_Position = projectionMatrix * viewMatrix * position;

    uv_frag_in = pos / uvScaleFactor;

    camera_position = vec3(viewMatrix * vec4(0.0, 0.0, 0.0, 1.0));
}
