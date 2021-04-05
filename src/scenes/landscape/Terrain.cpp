#include "Terrain.h"

#include <Image.h>
#include <ImageOps.h>
#include <array>
#include <imgui.h>

DEFINE_SHADER(landscape_NoiseLib)
DEFINE_SHADER(landscape_ScatterLib)

DEFINE_DEFAULT_SHADERS(landscape_Landscape)
DEFINE_TESS_CONTROL_SHADER(landscape_Landscape)
DEFINE_TESS_EVALUATION_SHADER(landscape_Landscape)

void Terrain::init() {
    terrainShader = CREATE_DEFAULT_SHADER(landscape_Landscape);
    terrainShader->attachTessControlShader(SHADER_CODE(landscape_LandscapeTcs));
    terrainShader->attachTessEvaluationShader(SHADER_CODE(landscape_LandscapeTes));
    terrainShader->attachShaderLib(SHADER_CODE(landscape_NoiseLib));
    terrainShader->attachShaderLib(SHADER_CODE(landscape_ScatterLib));
    terrainShader->bind();

    terrainVA = generatePoints(terrainShader);

    initTextures();
}

void Terrain::showGui() {
    ImGui::DragFloat3("Terrain Levels", reinterpret_cast<float *>(&levels), 0.001F);
    ImGui::DragFloat("Tesselation", &tessellation);
    ImGui::DragFloat("UV scale", &uvScaleFactor, 0.1F);
}

void Terrain::render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const ShaderToggles &shaderToggles) {
    terrainShader->bind();
    terrainVA->bind();

    glm::mat4 modelMatrix = createModelMatrix(modelPosition, modelRotation, modelScale);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(viewMatrix * modelMatrix)));
    terrainShader->setUniform("modelMatrix", modelMatrix);
    terrainShader->setUniform("viewMatrix", viewMatrix);
    terrainShader->setUniform("projectionMatrix", projectionMatrix);
    terrainShader->setUniform("normalMatrix", normalMatrix);

    terrainShader->setUniform("tessellation", tessellation);

    terrainParams.setShaderUniforms(terrainShader);

    terrainShader->setUniform("grassLevel", levels.grassLevel);
    terrainShader->setUniform("rockLevel", levels.rockLevel);
    terrainShader->setUniform("blur", levels.blur);

    terrainShader->setUniform("showUVs", shaderToggles.showUVs);
    terrainShader->setUniform("showNormals", shaderToggles.showNormals);
    terrainShader->setUniform("showTangents", shaderToggles.showTangents);
    terrainShader->setUniform("useAtmosphericScattering", shaderToggles.useAtmosphericScattering);
    terrainShader->setUniform("useACESFilm", shaderToggles.useACESFilm);
    terrainShader->setUniform("uvScaleFactor", uvScaleFactor);

    terrainShader->setUniform("grassTexture", grassTexture, GL_TEXTURE0);
    terrainShader->setUniform("dirtTexture", dirtTexture, GL_TEXTURE1);
    terrainShader->setUniform("rockTexture", rockTexture, GL_TEXTURE2);

    if (shaderToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    GL_Call(glPatchParameteri(GL_PATCH_VERTICES, 3));
    GL_Call(glDrawElements(GL_PATCHES, terrainVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    if (shaderToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    terrainVA->unbind();
    terrainShader->unbind();
}

void Terrain::showLayersGui() { terrainParams.showLayersGui(); }

std::shared_ptr<Texture> createTextureFromImage(const Image &image) {
    auto texture = std::make_shared<Texture>();
    image.applyToTexture(texture);

    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    return texture;
}

std::shared_ptr<Texture> loadTexture(const std::string &fileName) {
    Image image;
    if (!ImageOps::load("landscape_resources/assets/textures/" + fileName, image)) {
        ImageOps::createCheckerBoard(image);
    }

    return createTextureFromImage(image);
}

void Terrain::initTextures() {
    const std::array<std::string, 3> fileNames = {
          "Ground037_1K_Color.png", //
          "Ground039_1K_Color.png", //
          "Ground022_1K_Color.png", //
    };

#define LOAD_TEXTURES_PARALLEL 1
#if LOAD_TEXTURES_PARALLEL
    std::array<Image, 3> images = {};
#pragma omp parallel for
    for (int i = 0; i < images.size(); i++) {
        if (!ImageOps::load("landscape_resources/assets/textures/" + fileNames[i], images[i])) {
            ImageOps::createCheckerBoard(images[i]);
        }
    }

    grassTexture = createTextureFromImage(images[0]);
    dirtTexture = createTextureFromImage(images[1]);
    rockTexture = createTextureFromImage(images[2]);
#else
    grassTexture = loadTexture(fileNames[0]);
    dirtTexture = loadTexture(fileNames[1]);
    rockTexture = loadTexture(fileNames[2]);
#endif
}

std::shared_ptr<VertexArray> Terrain::generatePoints(const std::shared_ptr<Shader> &shader) {
    auto result = std::make_shared<VertexArray>(shader);

    std::vector<float> quadVertices = {
          0.0F, 0.0F, //
          1.0F, 0.0F, //
          1.0F, 1.0F, //
          0.0F, 1.0F, //
    };

    // generate a 10x10 grid of points in the range of -500 to 500
    int width = 10;
    int height = 10;
    std::vector<float> vertices = {};
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            for (int i = 0; i < static_cast<int64_t>(quadVertices.size()); i++) {
                float f = quadVertices[i];
                if (i % 2 == 0) {
                    f += static_cast<float>(row);
                    f -= 5.0F;
                    f *= 100.0F;
                } else if (i % 2 == 1) {
                    f += static_cast<float>(col);
                    f -= 5.0F;
                    f *= 100.0F;
                }
                vertices.push_back(f);
            }
        }
    }

    BufferLayout bufferLayout = {
          {ShaderDataType::Float2, "position_in"},
    };
    auto buffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    result->addVertexBuffer(buffer);

    std::vector<glm::ivec3> indices = {};
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            const int i = (row * width + col) * 4;
            indices.emplace_back(i, i + 1, i + 2);
            indices.emplace_back(i, i + 2, i + 3);
        }
    }
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    result->setIndexBuffer(indexBuffer);

    return result;
}
