#pragma once

struct ShaderToggles {
    bool showNormals = false;
    bool showTangents = false;
    bool showUVs = false;
    bool drawWireframe = false;
    bool useAtmosphericScattering = false;
    bool useACESFilm = false;
    bool useAmbientOcclusion = false;
    float exposure = 1.0F;
    float gamma = 1.0F;
};
