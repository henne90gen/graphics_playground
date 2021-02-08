#version 330 core

struct DirLight {
    vec3 direction;
    vec3 color;
    float power;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float power;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

in vec2 uv_frag_in;
in vec3 normal_frag_in;
in vec3 tangent_frag_in;
in vec3 bitangent_frag_in;
in vec3 model_position;
in float normalized_height;

in vec3 extinction;
in vec3 inScatter;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gExtinction;
layout (location = 4) out vec4 gInScatter;
layout (location = 5) out vec4 gDoLighting;

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform vec3 cameraPosition;

uniform bool showNormals;
uniform bool showTangents;
uniform bool showUVs;
uniform bool useAtmosphericScattering;
uniform bool useACESFilm;

uniform float grassLevel;
uniform float rockLevel;
uniform float blur;

uniform vec3 lightPosition;
uniform vec3 sunDirection;
uniform vec3 lightColor;
uniform float lightPower;

uniform sampler2D grassTexture;
uniform sampler2D dirtTexture;
uniform sampler2D rockTexture;

// https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
const float PHI = 1.61803398874989484820459;// = Golden Ratio
float gold_noise(in vec2 uv, in float seed) {
    return fract(tan(distance(uv * PHI, uv) * seed) * uv.x);
}
float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}


// -------------------- Begin -----------------------
// https://www.shadertoy.com/view/4slGD4
const vec2 add = vec2(1.0, 0.0);
#define HASHSCALE1 .1031
#define HASHSCALE3 vec3(.1031, .1030, .0973)
#define HASHSCALE4 vec4(1031, .1030, .0973, .1099)
float Hash12(vec2 p) {
    vec3 p3  = fract(vec3(p.xyx) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}
vec2 Hash22(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * HASHSCALE3);
    p3 += dot(p3, p3.yzx+19.19);
    return fract((p3.xx+p3.yz)*p3.zy);
}
float Noise(in vec2 x) {
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float res = mix(mix(Hash12(p), Hash12(p + add.xy), f.x),
    mix(Hash12(p + add.yx), Hash12(p + add.xx), f.x), f.y);
    return res;
}
vec2 Noise2(in vec2 x) {
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y * 57.0;
    vec2 res = mix(mix(Hash22(p), Hash22(p + add.xy), f.x),
    mix(Hash22(p + add.yx), Hash22(p + add.xx), f.x), f.y);
    return res;
}

void DoLighting(inout vec3 mat, in vec3 pos, in vec3 normal, in vec3 eyeDir, in float dis, in float ambient, in float specular) {
    float h = dot(normalize(sunDirection), normal);
    float c = max(h, 0.0)+ambient;
    mat = mat * lightColor * c;
    // Specular...
    if (h > 0.0)
    {
        vec3 R = reflect(normalize(sunDirection), normal);
        float specAmount = pow(max(dot(R, normalize(eyeDir)), 0.0), 3.0)*specular;
        mat = mix(mat, lightColor, specAmount);
    }
}

vec3 getTerrainColor(vec3 pos, vec3 normal, float height, float dis) {
    vec3 mat;
    float specular = 0.0F;
    float ambient = 0.1F;
    vec3 dir = normalize(pos - cameraPosition);

    vec3 matPos = pos * 2.0;// ... I had change scale halfway though, this lazy multiply allow me to keep the graphic scales I had

    float disSqrd = dis * dis;// Squaring it gives better distance scales.

    float f = clamp(Noise(matPos.xz*.05), 0.0, 1.0);//*10.8;
    f += Noise(matPos.xz*.1+normal.yz*1.08)*.85;
    f *= .55;
    vec3 m = mix(vec3(.63*f+.2, .7*f+.1, .7*f+.1), vec3(f*.43+.1, f*.3+.2, f*.35+.1), f*.65);
    mat = m*vec3(f*m.x+.36, f*m.y+.30, f*m.z+.28);
    // Should have used smoothstep to add colours, but left it using 'if' for sanity...
    #if 0
    if (normal.y < .5) {
        float v = normal.y;
        float c = (.5-normal.y) * 4.0;
        c = clamp(c*c, 0.1, 1.0);
        f = Noise(vec2(matPos.x*.09, matPos.z*.095+matPos.yy*0.15));
        f += Noise(vec2(matPos.x*2.233, matPos.z*2.23))*0.5;
        mat = mix(mat, vec3(.4*f), c);
        specular+=.1;
    }
        #endif

    // Grass. Use the normal to decide when to plonk grass down...
    if (height < 1.0 && normal.y > 0.65) {
        m = vec3(Noise(matPos.xz*.023)*.5+.15, Noise(matPos.xz*.03)*.6+.25, 0.0);
        m *= (normal.y- 0.65)*.6;
        mat = mix(mat, m, clamp((normal.y-.65)*1.3 * (453.5-matPos.y)*0.1, 0.0, 1.0));
    }

        // TODO find out how this is calculated
        #if 0
    float treeCol = 0.0F;
    if (treeCol > 0.0) {
        mat = vec3(.02+Noise(matPos.xz*5.0)*.03, .05, .0);
        normal = normalize(normal+vec3(Noise(matPos.xz*33.0)*1.0-.5, .0, Noise(matPos.xz*33.0)*1.0-.5));
        specular = .0;
    }
        #endif

    // Snow topped mountains...
    if (height > 1.5 && normal.y > .42) {
        float snow = clamp((matPos.y - 80.0 - Noise(matPos.xz * .1)*28.0) * 0.035, 0.0, 1.0);
        mat = mix(mat, vec3(.7, .7, .8), snow);
        specular += snow;
        ambient+=snow *.3;
    }

        // Beach effect...
        #if 1
    if (height < 0.5) {
        if (normal.y > .4)
        {
            f = Noise(matPos.xz * .084)*1.5;
            f = clamp((1.45-f-matPos.y) * 1.34, 0.0, .67);
            float t = (normal.y-.4);
            t = (t*t);
            mat = mix(mat, vec3(.09+t, .07+t, .03+t), f);
        }
        // Cheap under water darkening...it's wet after all...
        if (matPos.y < 0.0)
        {
            mat *= .2;
        }
    }
        #endif

    DoLighting(mat, pos, normal, dir, disSqrd, ambient, specular);

    // Do the water...
    #if 0
    if (matPos.y < 0.0) {
        // Pull back along the ray direction to get water surface point at y = 0.0 ...
        float time = (iTime)*.03;
        vec3 watPos = matPos;
        watPos += -dir * (watPos.y/dir.y);
        // Make some dodgy waves...
        float tx = cos(watPos.x*.052) *4.5;
        float tz = sin(watPos.z*.072) *4.5;
        vec2 co = Noise2(vec2(watPos.x*4.7+1.3+tz, watPos.z*4.69+time*35.0-tx));
        co += Noise2(vec2(watPos.z*8.6+time*13.0-tx, watPos.x*8.712+tz))*.4;
        vec3 nor = normalize(vec3(co.x, 20.0, co.y));
        nor = normalize(reflect(dir, nor));//normalize((-2.0*(dot(dir, nor))*nor)+dir);
        // Mix it in at depth transparancy to give beach cues..
        tx = watPos.y-matPos.y;
        mat = mix(mat, GetClouds(GetSky(nor)*vec3(.3, .3, .5), nor)*.1+vec3(.0, .02, .03), clamp((tx)*.4, .6, 1.));
        // Add some extra water glint...
        mat += vec3(.1)*clamp(1.-pow(tx+.5, 3.)*texture(iChannel1, watPos.xz*.1, -2.).x, 0., 1.0);
        float sunAmount = max(dot(nor, sunLight), 0.0);
        mat = mat + sunColour * pow(sunAmount, 228.5)*.6;
        vec3 temp = (watPos-cameraPos*2.)*.5;
        disSqrd = dot(temp, temp);
    }
    mat = ApplyFog(mat, disSqrd, dir);
    #endif
    return mat;
}

// -------------------- End -------------------------

// --------------------------------------------------
// https://www.shadertoy.com/view/3tjBDm
float get_occlusion_factor(vec3 normal, vec3 sight_dir) {
    return mix(abs(dot(sight_dir, normal)), (1.0 - pow(mix(abs(normal.z), 1.0, 0.3), 25.0)), 0.4);
}

float get_specular_factor(vec2 uv, vec3 normal, vec3 sight_dir) {
    float occlusion_factor = 1.0 - get_occlusion_factor(normal, sight_dir);
    // TODO    float texture_factor = texture(iChannel1, uv * 0.9 + vec2(0.5)).x;
    float texture_factor = 1.0F;
    return pow(texture_factor, 2.0) * pow(occlusion_factor, 5.0);
}

vec3 get_diffuse_color(vec2 uv, vec3 normal, vec3 sight_dir) {
    vec3 base = vec3(0.02, 0.015, 0.005) * 0.5;
    vec3 middle = vec3(0.1, 0.2, 0.0);
    vec3 top = vec3(0.2, 0.2, 0.1);

    float occlusion_factor = 1.0 - pow(1.0 - get_occlusion_factor(normal, sight_dir), 2.0);

    // TODO   float base_factor = (1.0 - texture(iChannel1, uv).x) * 2.0;
    float base_factor = (1.0F - rand(uv)) * 2.0F;
    base_factor = clamp(base_factor - occlusion_factor, 0.0, 1.0);
    base_factor = pow(base_factor, 0.5);

    // TODO    float top_factor = texture(iChannel1, uv).x * 1.5;
    float top_factor = rand(uv) * 1.5F;
    top_factor = clamp(top_factor - occlusion_factor, 0.0, 1.0);
    top_factor = pow(top_factor, 1.0);

    vec3 color = mix(base, middle, base_factor);
    color = mix(color, top, top_factor);
    return color;
}

float light_ambient(vec2 uv, vec3 normal, vec3 sight_dir) {
    // TODO   float ao_original = sample_noise(iChannel1, uv).x;
    float ao_original = rand(uv);
    //    ao_original = 1.0F;
    float ao_decay = pow(get_occlusion_factor(normal, sight_dir), 2.0);
    return mix(1.0, ao_original, ao_decay);
}

float light_diffuse(vec3 normal, vec3 light_dir, float scattering) {
    float result = clamp(dot(-light_dir, normal) * (1.0 - scattering) + scattering, 0.0, 1.0);
    return result;
}

float light_specular(vec3 normal, vec3 light_dir, vec3 sight_dir, float shininess, float scattering) {
    vec3 reflected = reflect(light_dir, normal);
    float result = max(dot(-sight_dir, reflected), 0.0);
    result *= max(sign(dot(normal, -light_dir)), 0.0);
    result = max(result * (1.0 - scattering) + scattering, 0.0);
    result = pow(result, shininess);
    return result;
}

vec3 render_grass(vec3 normal, vec2 uv, vec3 sight_dir, vec3 light_dir, vec3 light_color, vec3 ambient_color) {
    //    vec3 noisy_normal = get_noisy_normal(normal, uv);
    vec3 noisy_normal = normal;
    vec3 color = get_diffuse_color(uv, normal, sight_dir);

    float ambient = light_ambient(uv, noisy_normal, sight_dir) * 1.0;

    float diffuse = light_diffuse(noisy_normal, light_dir, 0.1) * 1.0;
    diffuse *= 1.0 + pow(1.0 - get_occlusion_factor(normal, sight_dir), 5.0) * 0.3;

    float specular = light_specular(noisy_normal, light_dir, sight_dir, 2.0, 0.0) * 0.5;
    specular *= get_specular_factor(uv, noisy_normal, sight_dir);

    color *= (ambient * ambient_color + diffuse * light_color);
    color += vec3(1.0, 1.0, 0.1) * light_color * specular;
    return color;
}

// -------------------- End -------------------------

vec3 getSurfaceColor(float height) {
    // TODO make color dependent on normal
    const float grassDamper = 0.75F;
    const float dirtDamper = 0.9F;
    if (height < grassLevel-blur) {
        vec3 grassColor = texture(grassTexture, uv_frag_in).rgb * grassDamper;
        return grassColor;
    } else if (height < grassLevel+blur) {
        vec3 grassColor = texture(grassTexture, uv_frag_in).rgb * grassDamper;
        vec3 dirtColor = texture(dirtTexture, uv_frag_in).rgb * dirtDamper;
        float t = (height-(grassLevel-blur)) / (2.0F*blur);
        return mix(grassColor, dirtColor, t);
    } else if (height < rockLevel-blur){
        vec3 dirtColor = texture(dirtTexture, uv_frag_in).rgb * dirtDamper;
        return dirtColor;
    } else if (height < rockLevel+blur) {
        vec3 dirtColor = texture(dirtTexture, uv_frag_in).rgb * dirtDamper;
        vec3 rockColor = texture(rockTexture, uv_frag_in).rgb;
        float t = (height-(rockLevel-blur)) / (2.0F*blur);
        return mix(dirtColor, rockColor, t);
    } else {
        vec3 rockColor = texture(rockTexture, uv_frag_in).rgb;
        return rockColor;
    }
}

vec4 debugColors(vec3 normal, vec3 tangent, vec3 bitangent) {
    if (showUVs) {
        // this uv is only used for visualization (it shows the borders between neighboring textures)
        vec2 uv = fract(uv_frag_in);
        return vec4(uv, 0.0F, 1.0F);
    }

    if (showNormals) {
        normal += 1.0F;
        normal /= 2.0F;
        return vec4(normal, 1.0F);
    }

    if (showTangents) {
        tangent += 1.0F;
        tangent /= 2.0F;

        bitangent += 1.0F;
        bitangent /= 2.0F;

        return vec4(tangent.y, bitangent.y, 0.0F, 1.0F);
    }

    return vec4(1.0F, 1.0F, 1.0F, 1.0F);
}

void main() {
    gPosition = vec4(model_position, 1.0F);
    gNormal = vec4(normalize(normalMatrix * normal_frag_in), 1.0F);
    gAlbedoSpec = vec4(getSurfaceColor(normalized_height), 1.0F);
    gExtinction = vec4(extinction, 1.0F);
    gInScatter = vec4(inScatter, 1.0F);
    gDoLighting = vec4(1.0F);
}
