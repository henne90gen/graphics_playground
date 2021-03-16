#pragma once

struct ShaderToggles {
    bool showNormals = false;
    bool showTangents = false;
    bool showUVs = false;
    bool drawWireframe = false;
    bool useAtmosphericScattering = true;
    bool useACESFilm = false;
    bool useAmbientOcclusion = false;
    float exposure = 1.6F;
    float gamma = 0.6F;
};
