#include "Trees.h"

#include <array>
#include <imgui.h>
#include <random>
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
    ImGui::DragFloat("Tree Scale", &treeScale);
    treeSettings.showGui();
#if USE_TREE_MODELS
    ImGui::Text("Mesh count: %zu", treeModel.getMeshes().size());
    ImGui::Text("Vertex count: %d", vertexCount);
#endif
}

void Trees::render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const ShaderToggles &shaderToggles,
                   const TerrainParams &terrainParams) {
    if (!showTrees) {
        return;
    }

    if (usingGeneratedTrees) {
        generateTrees();
        renderGeneratedTrees(projectionMatrix, viewMatrix, shaderToggles, terrainParams);
    } else {
        // TODO(henne): compute shader execution can be moved into init
        renderComputeShader(terrainParams);

#if USE_TREE_MODELS
        renderTreeModels(projectionMatrix, viewMatrix, shaderToggles);
#else
        renderCubes(projectionMatrix, viewMatrix, shaderToggles, terrainParams);
#endif

        renderGrid(projectionMatrix, viewMatrix);
    }
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
    if (!showGrid) {
        return;
    }

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
                             const ShaderToggles &shaderToggles) {
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

void appendLeaf(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<glm::ivec3> &indices,
                const glm::mat4 &modelMatrix) {
    const std::array<glm::vec3, 11> leafVertices = {
          glm::vec3(-0.1F, 0.0F, 0.0F), //
          glm::vec3(0.1F, 0.0F, 0.0F),  //
          glm::vec3(0.1F, 0.0F, 1.0F),  //
          glm::vec3(-0.1F, 0.0F, 1.0F), //
          glm::vec3(-0.4F, 0.0F, 0.5F), //
          glm::vec3(-1.0F, 0.0F, 0.8F), //
          glm::vec3(-1.0F, 0.0F, 1.7F), //
          glm::vec3(0.0F, 0.0F, 2.5F),  //
          glm::vec3(1.0F, 0.0F, 1.7F),  //
          glm::vec3(1.0F, 0.0F, 0.8F),  //
          glm::vec3(0.4F, 0.0F, 0.5F),  //
    };
    const glm::vec3 quadNormal = {0.0F, 1.0F, 0.0F};

    auto positionOffset = positions.size();
    for (const auto &leafVertex : leafVertices) {
        positions.emplace_back(modelMatrix * glm::vec4(leafVertex, 1.0F));
        normals.emplace_back(modelMatrix * glm::vec4(quadNormal, 0.0F));
    }

    indices.emplace_back(positionOffset + 0, positionOffset + 1, positionOffset + 2);
    indices.emplace_back(positionOffset + 0, positionOffset + 2, positionOffset + 3);
    indices.emplace_back(positionOffset + 4, positionOffset + 3, positionOffset + 5);
    indices.emplace_back(positionOffset + 5, positionOffset + 3, positionOffset + 6);
    indices.emplace_back(positionOffset + 6, positionOffset + 3, positionOffset + 7);
    indices.emplace_back(positionOffset + 7, positionOffset + 3, positionOffset + 2);
    indices.emplace_back(positionOffset + 7, positionOffset + 2, positionOffset + 8);
    indices.emplace_back(positionOffset + 8, positionOffset + 2, positionOffset + 9);
    indices.emplace_back(positionOffset + 9, positionOffset + 2, positionOffset + 10);
}

void Trees::generateTrees() {
    if (generatedTreesVA != nullptr) {
        for (auto &vb : generatedTreesVA->getVertexBuffers()) {
            const unsigned int glid = vb->getGLID();
            GL_Call(glDeleteBuffers(1, &glid));
        }
        generatedTreesVA->getVertexBuffers().clear();

        const unsigned int glid = generatedTreesVA->getIndexBuffer()->getGLID();
        GL_Call(glDeleteBuffers(1, &glid));
        generatedTreesVA->getIndexBuffer() = nullptr;
    } else {
        generatedTreesVA = std::make_shared<VertexArray>(flatColorShader);
    }

    std::vector<glm::vec3> positions = {};
    std::vector<glm::vec3> normals = {};
    std::vector<glm::ivec3> indices = {};
    Tree *tree = Tree::create(treeSettings);
    tree->construct(positions, normals, indices);

    std::vector<glm::mat4> leafModelMatrices = {};
    tree->addLeaves(leafModelMatrices);

    int leafPositionOffset = positions.size();
    int leafIndicesOffset = indices.size();

    int sectorCount = 5;
    int stackCount = 3;
    int verticesPerLeaf = (sectorCount + 1) * (stackCount + 1);
    int indicesPerLeaf = (stackCount - 1) * sectorCount * 2;
    int totalVertices = leafPositionOffset + leafModelMatrices.size() * verticesPerLeaf;
    int totalIndices = leafIndicesOffset + leafModelMatrices.size() * indicesPerLeaf;

    positions.resize(totalVertices);
    normals.resize(totalVertices);
    indices.resize(totalIndices);

#pragma omp parallel for
    for (int i = 0; i < leafModelMatrices.size(); i++) {
        const auto &modelMatrix = leafModelMatrices[i];
#if 0
        appendLeaf(positions, normals, indices, modelMatrix);
#else
        std::vector<glm::vec3> vertices = {};
        std::vector<glm::ivec3> sphereIndices = {};
        appendSphere(vertices, sphereIndices, 5, 3);
        int positionOffset = leafPositionOffset + i * verticesPerLeaf;
        int indexOffset = leafIndicesOffset + i * indicesPerLeaf;
        for (int j = 0; j < vertices.size(); j++) {
            const auto &vertex = vertices[j];
            positions[positionOffset + j] = glm::vec3(modelMatrix * glm::vec4(vertex, 1.0));
            normals[positionOffset + j] = glm::vec3(modelMatrix * glm::vec4(0.0F, 1.0F, 0.0F, 0.0F));
        }
        for (int j = 0; j < sphereIndices.size(); j++) {
            const auto &index = sphereIndices[j];
            indices[indexOffset + j] = index + positionOffset;
        }
#endif
    }

    std::vector<float> vertexData = {};
    for (int i = 0; i < positions.size(); i++) {
        vertexData.push_back(positions[i].x);
        vertexData.push_back(positions[i].y);
        vertexData.push_back(positions[i].z);
        vertexData.push_back(normals[i].x);
        vertexData.push_back(normals[i].y);
        vertexData.push_back(normals[i].z);
    }

    BufferLayout layout = {
          {ShaderDataType::Float3, "a_Position"},
          {ShaderDataType::Float3, "a_Normal"},
    };
    auto vertexBuffer = std::make_shared<VertexBuffer>(vertexData, layout);
    generatedTreesVA->addVertexBuffer(vertexBuffer);

    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    generatedTreesVA->setIndexBuffer(indexBuffer);
}

void Trees::renderGeneratedTrees(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
                                 const ShaderToggles &shaderToggles, const TerrainParams &terrainParams) {
    generatedTreesVA->bind();
    flatColorShader->bind();
    generatedTreesVA->setShader(flatColorShader);
    auto modelMatrix = glm::identity<glm::mat4>();
    modelMatrix = glm::scale(modelMatrix, glm::vec3(treeScale));
    flatColorShader->setUniform("modelMatrix", modelMatrix);
    flatColorShader->setUniform("viewMatrix", viewMatrix);
    flatColorShader->setUniform("projectionMatrix", projectionMatrix);
    flatColorShader->setUniform("flatColor", glm::vec3(1.0F, 0.0F, 0.0F));

    if (shaderToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

#if 0
    GL_Call(glDrawElementsInstanced(GL_TRIANGLES, generatedTreesVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT,
                                    nullptr, treeCount));
#else
    GL_Call(glDrawElements(GL_TRIANGLES, generatedTreesVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
#endif

    if (shaderToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }
}
