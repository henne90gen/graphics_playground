#include "Trees.h"

#include <imgui.h>
#include <util/RenderUtils.h>

DEFINE_DEFAULT_SHADERS(landscape_Tree)
DEFINE_DEFAULT_SHADERS(landscape_Texture)
DEFINE_DEFAULT_SHADERS(landscape_FlatColor)

DEFINE_SHADER(landscape_NoiseLib)
DEFINE_SHADER(landscape_TreeComp)

void Trees::init() {
    shader = CREATE_DEFAULT_SHADER(landscape_Tree);
    shader->attachShaderLib(SHADER_CODE(landscape_NoiseLib));

    initComputeShader();

    initModel();
    initGrid();
}

void Trees::showGui() {
    ImGui::DragInt("Tree Count", &treeCount);
    ImGui::DragFloat("LOD Size", &lodSize);
    ImGui::DragFloat("LOD Inner Size", &lodInnerSize);
    ImGui::DragFloat("Grid Height", &gridHeight);
#if USE_TREE_MODELS
    ImGui::Text("Mesh count: %zu", treeModel.getMeshes().size());
    ImGui::Text("Vertex count: %d", vertexCount);
#endif
}

void Trees::render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const ShaderToggles &shaderToggles,
                   const TerrainParams &terrainParams) {
    // TODO(henne): compute shader execution can be moved into init
    renderComputeShader(terrainParams);

    renderTreeModels(projectionMatrix, viewMatrix, shaderToggles, terrainParams);
    renderCubes(projectionMatrix, viewMatrix, shaderToggles, terrainParams);

    renderGrid(projectionMatrix, viewMatrix);
}

void Trees::initComputeShader() {
    compShader = std::make_shared<Shader>();
    compShader->attachComputeShader(SHADER_CODE(landscape_TreeComp));
    compShader->attachShaderLib(SHADER_CODE(landscape_NoiseLib));

    GL_Call(glGenTextures(1, &treePositionTextureId));
    GL_Call(glActiveTexture(GL_TEXTURE0));
    GL_Call(glBindTexture(GL_TEXTURE_2D, treePositionTextureId));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, treePositionTextureWidth, treePositionTextureHeight, 0, GL_RGBA,
                         GL_FLOAT, nullptr));
    GL_Call(glBindImageTexture(0, treePositionTextureId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
}

void Trees::renderComputeShader(const TerrainParams &terrainParams) {
    compShader->bind();
    compShader->setUniform("treeCount", treeCount);
    compShader->setUniform("lodSize", lodSize);
    compShader->setUniform("lodInnerSize", lodInnerSize);
    terrainParams.setShaderUniforms(compShader);
    GL_Call(glBindImageTexture(0, treePositionTextureId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F));
    GL_Call(glDispatchCompute(4, 4, 1));
    GL_Call(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
}

void Trees::initGrid() {
    flatColorShader = CREATE_DEFAULT_SHADER(landscape_FlatColor);
    gridVA = std::make_shared<VertexArray>(flatColorShader);
    std::vector<glm::vec3> vertices = {};
    vertices.emplace_back(0.0F, 0.0F, 0.0F);
    vertices.emplace_back(1.0F, 0.0F, 0.0F);
    vertices.emplace_back(1.0F, 0.0F, 1.0F);
    vertices.emplace_back(0.0F, 0.0F, 1.0F);

    BufferLayout layout = {{ShaderDataType::Float3, "a_Position"}};
    auto vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
    gridVA->addVertexBuffer(vertexBuffer);

    std::vector<unsigned int> indices = {};
    indices.emplace_back(0);
    indices.emplace_back(1);
    indices.emplace_back(1);
    indices.emplace_back(2);
    indices.emplace_back(2);
    indices.emplace_back(3);
    indices.emplace_back(3);
    indices.emplace_back(0);
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    gridVA->setIndexBuffer(indexBuffer);
}

void Trees::renderGrid(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
    gridVA->bind();
    flatColorShader->bind();
    flatColorShader->setUniform("viewMatrix", viewMatrix);
    flatColorShader->setUniform("projectionMatrix", projectionMatrix);
    flatColorShader->setUniform("flatColor", glm::vec3(1.0F));

    const float lodH = lodSize / 2.0F;
    const float lodIH = lodInnerSize / 2.0F;
    const float smallSideLength = (lodSize - lodInnerSize) / 2.0F;
    std::vector<glm::vec4> gridOffsets = {
          {lodInnerSize / -2.0F, lodInnerSize / -2.0F, lodInnerSize, lodInnerSize},

          {-lodH, lodIH, smallSideLength, smallSideLength},
          {-lodIH, lodIH, lodInnerSize, smallSideLength},
          {lodIH, lodIH, smallSideLength, smallSideLength},

          {-lodH, -lodIH, smallSideLength, lodInnerSize},
          {lodIH, -lodIH, smallSideLength, lodInnerSize},

          {-lodH, -lodH, smallSideLength, smallSideLength},
          {-lodIH, -lodH, lodInnerSize, smallSideLength},
          {lodIH, -lodH, smallSideLength, smallSideLength},
    };
    for (const auto &gridOffset : gridOffsets) {
        auto modelMatrix = glm::identity<glm::mat4>();
        modelMatrix = glm::translate(modelMatrix, glm::vec3(gridOffset.x, gridHeight, gridOffset.y));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(gridOffset.z, 1.0F, gridOffset.w));
        flatColorShader->setUniform("modelMatrix", modelMatrix);
        GL_Call(glDrawElements(GL_LINES, gridVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    }
}

void Trees::initModel() {
#if USE_TREE_MODELS
#if 1
    unsigned int error =
          Model::loadFromFile("landscape_resources/assets/models/low_poly_tree/low_poly_tree.obj", shader, treeModel);
#else
    unsigned int error =
          Model::loadFromFile("landscape_resources/assets/models/MangoTree/tree_mango_var01.obj", shader, treeModel);
#endif
    if (error != 0) {
        std::cout << "Failed to load tree model" << std::endl;
        return;
    }

    for (const auto &mesh : treeModel.getRawModel().meshes) {
        vertexCount += mesh.vertices.size();
    }
#else
    cubeVA = createCubeVA(shader);
#endif
}

void Trees::renderTreeModels(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
                             const ShaderToggles &shaderToggles, const TerrainParams &terrainParams) {
#if USE_TREE_MODELS
    if (!treeModel.isLoaded()) {
        return;
    }

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(viewMatrix * modelMatrix)));
    shader->bind();
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("normalMatrix", normalMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    shader->setUniform("treeCount", treeCount);
    shader->setUniform("textureSampler", 0);
    shader->setUniform("positionTexture", 1);

    GL_Call(glActiveTexture(GL_TEXTURE1));
    GL_Call(glBindTexture(GL_TEXTURE_2D, treePositionTextureId));

    GL_Call(glActiveTexture(GL_TEXTURE0));
    for (const auto &mesh : treeModel.getMeshes()) {
        if (!mesh.visible) {
            continue;
        }

        mesh.vertexArray->bind();

        mesh.texture->bind();

        if (shaderToggles.drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        }

        GL_Call(
              glDrawElementsInstanced(GL_TRIANGLES, mesh.indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr, treeCount));

        if (shaderToggles.drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        mesh.vertexArray->unbind();
    }
#endif
}

void Trees::renderCubes(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
                        const ShaderToggles &shaderToggles, const TerrainParams &terrainParams) {
#if !USE_TREE_MODELS
    cubeVA->bind();
    shader->bind();
    cubeVA->setShader(shader);
    auto modelMatrix = glm::identity<glm::mat4>();
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("treeCount", treeCount);
    shader->setUniform("positionTexture", 0);
    terrainParams.setShaderUniforms(shader);

    GL_Call(glActiveTexture(GL_TEXTURE0));
    GL_Call(glBindTexture(GL_TEXTURE_2D, treePositionTextureId));

    if (shaderToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    GL_Call(glDrawElementsInstanced(GL_TRIANGLES, cubeVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr,
                                    treeCount));

    if (shaderToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }
#endif
}
