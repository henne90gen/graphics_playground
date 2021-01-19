const float PI = 3.14159265358979323846;
#if 0
vec3 bR = vec3(1.95e-4, 1.1e-3, 2.94e-3);
float bM = 4e-5;
const float g = 0.93F;
#else
vec3 bR = vec3(5.8e-6, 13.5e-6, 33.1e-6);
float bM = 210.0e-5;
const float g = 0.76F;
#endif

// http://developer.amd.com/wordpress/media/2012/10/GDC_02_HoffmanPreetham.pdf
// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky/simulating-colors-of-the-sky
vec3 calcFex(vec3 cameraPosition, vec3 modelPosition) {
    vec3 modelToCamera = cameraPosition - modelPosition;
    float s = length(modelToCamera);
    return exp(-(bR + bM) * s);
}

// http://developer.amd.com/wordpress/media/2012/10/GDC_02_HoffmanPreetham.pdf
// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky/simulating-colors-of-the-sky
vec3 calcLin(vec3 cameraPosition, vec3 modelPosition, vec3 lightDirection, vec3 lightColor, float lightPower) {
    vec3 modelToCamera = cameraPosition - modelPosition;
    float cosTheta = dot(modelToCamera, lightDirection) / (length(modelToCamera) * length(lightDirection));
    float cR = 3 / (16*PI);
    float cM = 1 / (4*PI);
    vec3 bRTheta = cR * bR * (1 + cosTheta*cosTheta);
    float oneMinusGSq = (1-g) * (1-g);
    float bMTheta = cM * bM * (oneMinusGSq / pow(1 + g*g - 2*g*cosTheta, 3/2));
    vec3 t1 = (bRTheta + bMTheta) / (bR + bM);
    vec3 t2 = 1 - calcFex(cameraPosition, modelPosition);

    return t1 * lightColor * lightPower * t2;
}


