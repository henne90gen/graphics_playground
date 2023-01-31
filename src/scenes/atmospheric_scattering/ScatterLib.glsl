const float PI = 3.14159265358979323846;
#if 0
const vec3 bR = vec3(1.95e-4, 1.1e-3, 2.94e-3);
const float bM = 4e-5;
const float g = 0.93F;
#else
const vec3 bR = vec3(5.8e-6, 13.5e-6, 33.1e-6);
const float bM = 210.0e-5;
const float g = 0.76F;
#endif

// http://developer.amd.com/wordpress/media/2012/10/GDC_02_HoffmanPreetham.pdf
// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky/simulating-colors-of-the-sky
void calcScattering(vec3 cameraPosition, vec3 modelPosition, vec3 lightDirection, vec3 lightColor, float lightPower,
out vec3 extinction, out vec3 inScatter) {
    vec3 modelToCamera = cameraPosition - modelPosition;
    float s = length(modelToCamera);
    extinction = exp(-(bR + bM) * s);

    float cosTheta = dot(modelToCamera, lightDirection) / (length(modelToCamera) * length(lightDirection));
    float cR = 3.0F / (16.0F*PI);
    float cM = 1.0F / (4.0F*PI);
    vec3 bRTheta = cR * bR * (1.0F + cosTheta*cosTheta);
    float oneMinusGSq = (1.0F-g) * (1.0F-g);
    float bMTheta = cM * bM * (oneMinusGSq / pow(1.0F + g*g - 2.0F*g*cosTheta, 3.0F/2.0F));
    vec3 t1 = (bRTheta + bMTheta) / (bR + bM);
    vec3 t2 = 1.0F - extinction;
    inScatter = t1 * lightColor * lightPower * t2;
}


