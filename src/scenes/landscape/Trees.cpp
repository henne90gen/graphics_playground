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

    initComputeShaderStuff();

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

    initGrid();
}

void Trees::showGui() {
    ImGui::DragInt("Tree Count", &treeCount);
    ImGui::DragFloat3("LOD", reinterpret_cast<float *>(&lodSize));
    ImGui::DragFloat("Grid Height", &gridHeight);
#if USE_TREE_MODELS
    ImGui::Text("Mesh count: %zul", treeModel.getMeshes().size());
    ImGui::Text("Vertex count: %d", vertexCount);
#endif
}

void Trees::render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const ShaderToggles &shaderToggles,
                   const TerrainParams &terrainParams) {
    {
        // TODO(henne): compute shader execution can be moved into init
        compShader->bind();
        compShader->setUniform("treeCount", treeCount);
        compShader->setUniform("lod0Size", lodSize.x);
        compShader->setUniform("lod1Size", lodSize.y);
        compShader->setUniform("lod2Size", lodSize.z);
        terrainParams.setShaderUniforms(compShader);
        GL_Call(glBindImageTexture(0, treePositionTextureId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F));
        GL_Call(glDispatchCompute(treePositionTextureWidth, treePositionTextureHeight, 1));
        GL_Call(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
    }

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

    terrainParams.setShaderUniforms(shader);

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
#else
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

    renderGrid(projectionMatrix, viewMatrix);
}

void Trees::initComputeShaderStuff() {
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
    GL_Call(glBindImageTexture(0, treePositionTextureId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F));
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

    float lod0InnerSize = 100.0F;
    const float lod0H = lodSize.x / 2.0F;
    const float lod0IH = lod0InnerSize / 2.0F;
    const float smallSideLength = (lodSize.x - lod0InnerSize) / 2.0F;
    std::vector<glm::vec4> gridOffsets = {
          {lod0InnerSize / -2.0F, lod0InnerSize / -2.0F, lod0InnerSize, lod0InnerSize},
          {lodSize.x / -2.0F, lodSize.x / -2.0F, lodSize.x, lodSize.x},
          {lodSize.y / -2.0F, lodSize.y / -2.0F, lodSize.y, lodSize.y},
          {lodSize.z / -2.0F, lodSize.z / -2.0F, lodSize.z, lodSize.z},

          {-lod0H, lod0IH, smallSideLength, smallSideLength},
          {-lod0IH, lod0IH, lod0InnerSize, smallSideLength},
          {lod0IH, lod0IH, smallSideLength, smallSideLength},

          {-lod0H, -lod0IH, smallSideLength, lod0InnerSize},
          {lod0IH, -lod0IH, smallSideLength, lod0InnerSize},

          {-lod0H, -lod0H, smallSideLength, smallSideLength},
          {-lod0IH, -lod0H, lod0InnerSize, smallSideLength},
          {lod0IH, -lod0H, smallSideLength, smallSideLength},
    };
    for (const auto &gridOffset : gridOffsets) {
        auto modelMatrix = glm::identity<glm::mat4>();
        modelMatrix = glm::translate(modelMatrix, glm::vec3(gridOffset.x, gridHeight, gridOffset.y));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(gridOffset.z, 1.0F, gridOffset.w));
        flatColorShader->setUniform("modelMatrix", modelMatrix);
        GL_Call(glDrawElements(GL_LINES, gridVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    }
}
