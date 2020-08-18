#include "DtmViewer.h"

#include <future>
#include <glm/glm.hpp>

#include "util/ImGuiUtils.h"
#include "util/TimeUtils.h"
#include "util/VectorUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100000.0F;

void DtmViewer::setup() {
    simpleShader = std::make_shared<Shader>("scenes/dtm_viewer/PathVert.glsl", "scenes/dtm_viewer/PathFrag.glsl");
    shader = std::make_shared<Shader>("scenes/dtm_viewer/TerrainVert.glsl", "scenes/dtm_viewer/TerrainFrag.glsl");
    shader->bind();

    loadDtm();
}

void DtmViewer::destroy() {}

void DtmViewer::tick() {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto modelScale = glm::vec3(1.0F, 1.0F, 1.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto modelRotation = glm::vec3(0.0F, 0.0F, 0.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraPosition = glm::vec3(-120.0F, -155.0F, -375.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraRotation = glm::vec3(3.5F, -0.05F, 0.0F);
    static glm::vec3 surfaceToLight = {-4.5F, 7.0F, 0.0F};
    static glm::vec3 lightColor = {1.0F, 1.0F, 1.0F};
    static float lightPower = 13.0F;
    static bool wireframe = false;
    static bool drawTriangles = true;
    static int verticesPerFrame = 40000;
    static auto terrainSettings = DtmSettings();
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        cameraPosition = dtm.pointToLookAt + glm::vec3(0.0F, 800.0F, -2000.0F);
        cameraPosition *= -1.0F;
        cameraPosition.y *= -1.0F;
    }

    showSettings(modelScale, cameraPosition, cameraRotation, surfaceToLight, lightColor, lightPower, wireframe,
                 drawTriangles, verticesPerFrame, terrainSettings);

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, modelScale);
    glm::mat4 viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    renderTerrain(modelMatrix, viewMatrix, projectionMatrix, normalMatrix, surfaceToLight, lightColor, lightPower,
                  wireframe, drawTriangles, verticesPerFrame, terrainSettings);
    renderBoundingBox(modelMatrix, viewMatrix, projectionMatrix);
}

void DtmViewer::showSettings(glm::vec3 &modelScale, glm::vec3 &cameraPosition, glm::vec3 &cameraRotation,
                             glm::vec3 &lightPos, glm::vec3 &lightColor, float &lightPower, bool &wireframe,
                             bool &drawTriangles, int &verticesPerFrame, DtmSettings &terrainSettings) {
    const float dragSpeed = 0.01F;
    ImGui::Begin("Settings");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), dragSpeed);

    cameraPosition.x *= -1.0F;
    cameraPosition.z *= -1.0F;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition));
    cameraPosition.x *= -1.0F;
    cameraPosition.z *= -1.0F;

    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), dragSpeed);
    ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&lightPos));
    ImGui::ColorEdit3("Light Color", reinterpret_cast<float *>(&lightColor), dragSpeed);
    ImGui::DragFloat("Light Power", &lightPower, dragSpeed);
    ImGui::Checkbox("Wireframe", &wireframe);
    ImGui::Checkbox("Draw Triangles", &drawTriangles);
    ImGui::SliderInt("Vertices Per Frame", &verticesPerFrame, 10, 100000);
    ImGui::DragFloat4("Terrain Levels", reinterpret_cast<float *>(&terrainSettings), dragSpeed);

    ImGui::End();
}

void DtmViewer::renderTerrain(const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix,
                              const glm::mat4 &projectionMatrix, const glm::mat3 &normalMatrix,
                              const glm::vec3 &surfaceToLight, const glm::vec3 &lightColor, const float lightPower,
                              const bool wireframe, const bool drawTriangles, const int verticesPerFrame,
                              const DtmSettings &levels) {
    RECORD_SCOPE_NAME("Render Terrain");

    shader->bind();
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("normalMatrix", normalMatrix);

    shader->setUniform("waterLevel", levels.waterLevel);
    shader->setUniform("grassLevel", levels.grassLevel);
    shader->setUniform("rockLevel", levels.rockLevel);
    shader->setUniform("blur", levels.blur);
    shader->setUniform("surfaceToLight", surfaceToLight);
    shader->setUniform("lightColor", lightColor);
    shader->setUniform("lightPower", lightPower / 100.0F);

    dtm.va->bind();

    if (wireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    if (drawTriangles) {
        GL_Call(glDrawElements(GL_TRIANGLES, dtm.va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    } else {
        GL_Call(glDrawElements(GL_POINTS, dtm.va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    }

    if (wireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    dtm.va->unbind();
    shader->unbind();
}

void DtmViewer::loadDtm() {
    auto pointCount = countLinesInDir("../../../gis_data/dtm");
    if (pointCount < 0) {
        std::cout << "Could not count points in dtm" << std::endl;
        return;
    }

    dtm.vertices = std::vector<glm::vec3>(pointCount);
    dtm.normals = std::vector<glm::vec3>(pointCount);
    dtm.indices = std::vector<glm::ivec3>(pointCount * 2);

    dtm.va = std::make_shared<VertexArray>(shader);
    dtm.va->bind();

    BufferLayout positionLayout = {{ShaderDataType::Float3, "position"}};
    dtm.vertexBuffer = std::make_shared<VertexBuffer>(nullptr, pointCount * 3, positionLayout);
    dtm.va->addVertexBuffer(dtm.vertexBuffer);

    BufferLayout normalLayout = {{ShaderDataType::Float3, "in_normal"}};
    dtm.normalBuffer = std::make_shared<VertexBuffer>(nullptr, pointCount * 3, normalLayout);
    dtm.va->addVertexBuffer(dtm.normalBuffer);

    dtm.indexBuffer = std::make_shared<IndexBuffer>(nullptr, pointCount * 6);
    dtm.va->setIndexBuffer(dtm.indexBuffer);

    std::async(&DtmViewer::loadDtmAsync, this);
}

void DtmViewer::loadDtmAsync() {
    bool success = loadXyzDir("../../../gis_data/dtm", [this](const std::vector<glm::vec3> &points) {
        std::cout << "Loaded terrain data from disk (" << points.size() << " points)" << std::endl;

#define GET_INDEX(x, y) dtm.vertexMap[(static_cast<long>(x) << 32) | (y)]

        float stepWidth = 20.0F;
        auto vertexOffsetBefore = dtm.vertexOffset;
        auto indexOffsetBefore = dtm.indexOffset;

#if 1
#pragma omp parallel for
        for (unsigned int i = 0; i < points.size(); i++) {
            const auto &vertex = points[i];

            int x = vertex.x / stepWidth;
            float y = vertex.y / stepWidth;
            int z = vertex.z / stepWidth;
            dtm.vertices[dtm.vertexOffset + i] = {x, y, z};
            dtm.set(x, z, y);

            GET_INDEX(x, z) = i;

            const float L = dtm.get(x - 1, z);
            const float R = dtm.get(x + 1, z);
            const float B = dtm.get(x, z - 1);
            const float T = dtm.get(x, z + 1);
            dtm.normals[i] = glm::normalize(glm::vec3(2 * (L - R), 4, 2 * (B - T)));
        }
        dtm.vertexOffset += points.size();
#endif

#if 1
        for (unsigned int i = 0; i < points.size(); i++) {
            int x = dtm.vertices[dtm.vertexOffset + i].x;
            int y = dtm.vertices[dtm.vertexOffset + i].z;
            float topH = dtm.get(x, y + 1);
            float rightH = dtm.get(x + 1, y);
            if (topH != 0.0F && rightH != 0.0F) {
                dtm.indices[dtm.indexOffset++] = glm::ivec3( //
                      GET_INDEX(x, y + 1),                   //
                      GET_INDEX(x + 1, y),                   //
                      GET_INDEX(x, y)                        //
                );
            }
            float bottomH = dtm.get(x, y - 1);
            float leftH = dtm.get(x - 1, y);
            if (bottomH != 0.0F && leftH != 0.0F) {
                dtm.indices[dtm.indexOffset++] = glm::ivec3( //
                      GET_INDEX(x - 1, y),                   //
                      GET_INDEX(x, y),                       //
                      GET_INDEX(x, y - 1)                    //
                );
            }
        }
#endif

#if 0
        int numSegments = std::ceil(dtm.vertices.size() / verticesPerFrame);
        int segment = counter % numSegments;
        int offset = segment * verticesPerFrame * sizeof(glm::vec3);
        size_t size = normals.size() * sizeof(glm::vec3);

        dtm.normalBuffer->bind();
        GL_Call(glBufferSubData(GL_ARRAY_BUFFER, offset, size, normals.data()));
#endif

        {
            const std::lock_guard<std::mutex> guard(uploadsMutex);
            UploadSlice slice = {vertexOffsetBefore, dtm.vertexOffset, indexOffsetBefore, dtm.indexOffset};
            uploads.push_back(slice);
        }

        std::cout << "Finished loading terrain data" << std::endl;
    });

    if (!success) {
        std::cout << "Could not load dtm" << std::endl;
        return;
    }
}

void DtmViewer::renderBoundingBox(const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix,
                                  const glm::mat4 &projectionMatrix) {
    simpleShader->bind();
    simpleShader->setUniform("modelMatrix", modelMatrix);
    simpleShader->setUniform("viewMatrix", viewMatrix);
    simpleShader->setUniform("projectionMatrix", projectionMatrix);

    bbVA->bind();

    GL_Call(glDrawElements(GL_LINE_STRIP, bbVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    bbVA->unbind();
    simpleShader->unbind();
}

void DtmViewer::initBoundingBox(const BoundingBox3 &bb) {
    bbVA = std::make_shared<VertexArray>(simpleShader);
    std::vector<glm::vec3> vertices = {
          {bb.min.x, bb.min.y, bb.min.z}, // 0 - - -
          {bb.max.x, bb.min.y, bb.min.z}, // 1 + - -
          {bb.min.x, bb.max.y, bb.min.z}, // 2 - + -
          {bb.max.x, bb.max.y, bb.min.z}, // 3 + + -
          {bb.min.x, bb.min.y, bb.max.z}, // 4 - - +
          {bb.max.x, bb.min.y, bb.max.z}, // 5 + - +
          {bb.min.x, bb.max.y, bb.max.z}, // 6 - + +
          {bb.max.x, bb.max.y, bb.max.z}, // 7 + + +
    };

    BufferLayout layout = {{ShaderDataType::Float3, "position"}};
    auto vb = std::make_shared<VertexBuffer>(vertices, layout);
    bbVA->addVertexBuffer(vb);

    std::vector<unsigned int> indices = {
          0, 1, 5, 4, 0,    // bottom layer
          2, 3, 7, 6, 2,    // top layer
          3, 1, 5, 7, 6, 4, // remaining edges
    };

    auto ib = std::make_shared<IndexBuffer>(indices);
    bbVA->setIndexBuffer(ib);
}
