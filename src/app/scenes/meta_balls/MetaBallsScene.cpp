#include "MetaBallsScene.h"

#include <glad/glad.h>
#include <glm/ext.hpp>

#include "marching_cubes/MarchingCubes.h"
#include "util/ImGuiUtils.h"
#include "util/OpenGLUtils.h"
#include "util/VectorUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

void MetaBallsScene::setup() {
    shader = std::make_shared<Shader>("scenes/marching_cubes/MarchingCubesVert.glsl",
                                      "scenes/marching_cubes/MarchingCubesFrag.glsl");
    shader->bind();
    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    surfaceVertexArray = std::make_shared<VertexArray>(shader);
    surfaceVertexArray->bind();

    surfaceVertexBuffer = std::make_shared<VertexBuffer>();
    surfaceVertexBuffer->bind();

    BufferLayout surfaceBufferLayout = {
          {ShaderDataType::Float3, "a_Position"},
    };
    surfaceVertexBuffer->setLayout(surfaceBufferLayout);
    surfaceVertexArray->addVertexBuffer(surfaceVertexBuffer);

    surfaceIndexBuffer = std::make_shared<IndexBuffer>();
    surfaceIndexBuffer->bind();
}

void MetaBallsScene::onAspectRatioChange() {
    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
}

void MetaBallsScene::destroy() {}

void showSettings(glm::vec3 &modelRotation, glm::ivec3 &dimensions, MetaBallsScene::MetaBallsFuncType &funcType,
                  glm::vec3 &position, float &radius, bool &drawWireframe) {
    const float dragSpeed = 0.1F;
    ImGui::Begin("Settings");
    ImGui::DragFloat3("Rotation", (float *)&modelRotation, dragSpeed);
    ImGui::DragInt3("Dimensions", (int *)&dimensions);
    ImGui::DragFloat3("Position", (float *)&position, dragSpeed);
    ImGui::DragFloat("Radius", &radius, dragSpeed);

    static const std::array<const char *, 3> items = {"EXP", "INVERSE_DIST", "TEST_SPHERE"};
    ImGui::Combo("MetaBallsScene Function", reinterpret_cast<int *>(&funcType), items.data(), items.size());

    ImGui::Checkbox("Draw Wireframe", &drawWireframe);
    ImGui::End();
}

void MetaBallsScene::tick() {
    static auto translation = glm::vec3(1.0F, -1.5F, -5.0F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto modelRotation = glm::vec3();
    static auto cameraRotation = glm::vec3(0.25F, 0.0F, 0.0F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static float scale = 0.1F;                                 // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static bool drawWireframe = false;
    static auto dimensions = glm::ivec3(30, 40, 20);
    static float radius = 6.0F;
    static auto position = glm::vec3(10.0F, 10.0F, 10.0F);
    static auto funcType = MetaBallsScene::INVERSE_DIST;

    const auto startPos = glm::vec3(10.0F);
    const auto endPos = glm::vec3(10.0F, 30.0F, 10.0F);
    static float t = 0.0F;
    static float animationSpeed = 0.5F;
    double timeDelta = getLastFrameTime();
    t += animationSpeed * timeDelta;
    auto dir = endPos - startPos;
    position = startPos + t * dir;
    if (t < 0.0F || t > 1.0F) {
        animationSpeed *= -1.0F;
    }

    showSettings(modelRotation, dimensions, funcType, position, radius, drawWireframe);

    {
        RECORD_SCOPE_NAME("Update");
        updateSurface(dimensions, funcType, position, radius);
    }

    {
        RECORD_SCOPE_NAME("Render");
        const float halfScale = -0.5F;
        glm::vec3 modelCenter = glm::vec3(dimensions) * halfScale;

        shader->bind();

        glm::mat4 modelMatrix = glm::mat4(1.0F);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
        modelMatrix = glm::translate(modelMatrix, modelCenter);
        glm::mat4 viewMatrix = createViewMatrix(translation, cameraRotation);
        shader->setUniform("u_Model", modelMatrix);
        shader->setUniform("u_View", viewMatrix);
        shader->setUniform("u_Projection", projectionMatrix);
        shader->setUniform("u_Dimensions", dimensions);

        drawSurface(drawWireframe);

        shader->unbind();
    }
}

void MetaBallsScene::drawSurface(const bool drawWireframe) const {
    surfaceVertexArray->bind();

    shader->setUniform("u_Offset", glm::vec3());

    if (drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    GL_Call(glDrawElements(GL_TRIANGLES, surfaceIndexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

    if (drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    surfaceVertexArray->unbind();
}

void MetaBallsScene::updateSurface(const glm::ivec3 &dimensions, MetaBallsFuncType funcType, const glm::vec3 &position,
                              const float &radius) {
    std::vector<glm::vec3> vertices = {};
    std::vector<glm::ivec3> indices = {};

    std::vector<MetaBall> metaballs = {};
    metaballs.push_back({glm::vec3(10.0F), radius});
    metaballs.push_back({position, radius});

    implicit_surface_func func;
    if (funcType == MetaBallsScene::EXP) {
        func = exp_func(metaballs);
    } else if (funcType == MetaBallsScene::INVERSE_DIST) {
        func = inverse_dist_func(metaballs);
    } else {
        const float radiusSq = radius * radius;
        func = [&position, &radiusSq](const glm::vec3 &pos) {
            glm::vec3 final = pos - position;
            return final.x * final.x + final.y * final.y + final.z * final.z - radiusSq;
        };
    }

    runMarchingCubes(dimensions, vertices, indices, func);

    surfaceVertexBuffer->update(vertices);
    surfaceIndexBuffer->update(indices);
}
