#include "MetaBallsScene.h"

#include <glad/glad.h>
#include <glm/ext.hpp>

#include "marching_cubes/MarchingCubes.h"

#include "Main.h"
#include "util/ImGuiUtils.h"
#include "util/OpenGLUtils.h"
#include "util/RenderUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

DEFINE_SCENE_MAIN(MetaBallsScene)
DEFINE_SHADER(meta_balls_MetaBalls)

void MetaBallsScene::setup() {
    shader = SHADER(meta_balls_MetaBalls);
    shader->bind();
    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    surfaceVertexArray = std::make_shared<VertexArray>(shader);

    surfaceVertexBuffer = std::make_shared<VertexBuffer>();
    BufferLayout surfaceBufferLayout = {
          {ShaderDataType::Float3, "position"},
    };
    surfaceVertexBuffer->setLayout(surfaceBufferLayout);
    surfaceVertexArray->addVertexBuffer(surfaceVertexBuffer);

    surfaceIndexBuffer = std::make_shared<IndexBuffer>();
    surfaceVertexArray->setIndexBuffer(surfaceIndexBuffer);

    bbVa = createBoundingBoxVA(shader);
}

void MetaBallsScene::onAspectRatioChange() {
    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
}

void MetaBallsScene::destroy() {}

void MetaBallsScene::tick() {
    static auto cameraPosition = glm::vec3(0.8F, -1.5F, -5.0F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraRotation = glm::vec3(0.25F, 0.0F, 0.0F);  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto modelRotation = glm::vec3();
    static float scale = 0.1F; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static bool drawWireframe = false;
    static auto dimensions = glm::ivec3(45, 40, 20);
    static float radius = 6.0F;
    static auto funcType = MetaBallsScene::EXP;
    static auto positionBigBall = glm::vec3(20.0F, 10.0F, 10.0F);
    static auto position1 = glm::vec3(10.0F, 10.0F, 10.0F);
    static auto position2 = glm::vec3(10.0F, 10.0F, 10.0F);

    // TODO(henne): add unmodified spheres for metaballs

    static float animationSpeed = 0.1F;
    double timeDelta = getLastFrameTime();

    {
        const auto startPos = glm::vec3(20.0F, 10.0F, 10.0F);
        const auto endPos = glm::vec3(20.0F, 30.0F, 10.0F);
        static float t = 0.0F;
        static float animationDir = 1.0F;
        t += animationSpeed * animationDir * timeDelta;
        auto dir = endPos - startPos;
        position1 = startPos + t * dir;
        if (t < 0.0F || t > 1.0F) {
            animationDir *= -1.0F;
        }
    }

    {
        const auto startPos = glm::vec3(10.0F, 20.0F, 10.0F);
        const auto endPos = glm::vec3(35.0F, 20.0F, 10.0F);
        static float t = 0.0F;
        static float animationDir = 1.0F;
        t += 2.0F * animationSpeed * animationDir * timeDelta;
        auto dir = endPos - startPos;
        position2 = startPos + t * dir;
        if (t < 0.0F || t > 1.0F) {
            animationDir *= -1.0F;
        }
    }

    {
        const float dragSpeed = 0.01F;
        ImGui::Begin("Settings");
        ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition), dragSpeed);
        ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&modelRotation), dragSpeed);
        ImGui::DragFloat("Scale", &scale, dragSpeed);
        ImGui::Checkbox("Draw Wireframe", &drawWireframe);

        ImGui::Separator();

        ImGui::DragInt3("Dimensions", reinterpret_cast<int *>(&dimensions));
        ImGui::DragFloat("Animation Speed", &animationSpeed, dragSpeed);
        ImGui::DragFloat("Radius", &radius, dragSpeed);
        static const std::array<const char *, 3> items = {"EXP", "INVERSE_DIST", "TEST_SPHERE"};
        ImGui::Combo("MetaBallsScene Function", reinterpret_cast<int *>(&funcType), items.data(), items.size());
        ImGui::DragFloat3("Position Big Ball", reinterpret_cast<float *>(&positionBigBall), 0.1F);
        ImGui::End();
    }

    {
        RECORD_SCOPE_NAME("Update");
        const float radiusSq = radius * radius;
        std::vector<MetaBall> metaballs = {};
        metaballs.push_back({positionBigBall, radiusSq});
        metaballs.push_back({position1, radiusSq});
        metaballs.push_back({position2, radiusSq / 2.0F});

        updateSurface(dimensions, funcType, metaballs);
    }

    const float halfScale = -0.5F;
    glm::vec3 modelCenter = glm::vec3(dimensions) * halfScale;
    glm::mat4 viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    viewMatrix = glm::scale(viewMatrix, glm::vec3(scale));
    auto modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::translate(modelMatrix, modelCenter);

    shader->bind();
    shader->setUniform("u_View", viewMatrix);
    shader->setUniform("u_Projection", projectionMatrix);
    shader->setUniform("u_Dimensions", dimensions);

    drawSurface(modelMatrix, drawWireframe);

    drawBoundingBox(modelMatrix, modelCenter, dimensions);
}

void MetaBallsScene::drawSurface(const glm::mat4 &modelMatrix, const bool drawWireframe) {
    RECORD_SCOPE_NAME("Render Metaballs");

    shader->bind();
    shader->setUniform("u_Model", modelMatrix);

    if (drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    surfaceVertexArray->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, surfaceIndexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));
    surfaceVertexArray->unbind();

    if (drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    shader->unbind();
}

void MetaBallsScene::drawBoundingBox(glm::mat4 modelMatrix, const glm::vec3 &modelCenter,
                                     const glm::ivec3 &dimensions) {
    RECORD_SCOPE_NAME("Render Bounding Box");

    modelMatrix = glm::translate(modelMatrix, modelCenter * -1.0F);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(dimensions));
    shader->bind();
    shader->setUniform("u_Model", modelMatrix);

    bbVa->bind();
    GL_Call(glDrawElements(GL_LINE_STRIP, bbVa->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    bbVa->unbind();

    shader->unbind();
}

void MetaBallsScene::updateSurface(const glm::ivec3 &dimensions, MetaBallsFuncType funcType,
                                   const std::vector<MetaBall> &metaballs) {
    std::vector<glm::vec3> vertices = {};
    std::vector<glm::ivec3> indices = {};

    implicit_surface_func func;
    if (funcType == MetaBallsScene::EXP) {
        func = exp_func(metaballs);
    } else if (funcType == MetaBallsScene::INVERSE_DIST) {
        func = inverse_dist_func(metaballs);
    } else {
        func = sphere_func(metaballs);
    }

    runMarchingCubes(dimensions, vertices, indices, func);

    surfaceVertexArray->bind();
    surfaceVertexBuffer->update(vertices);
    surfaceIndexBuffer->update(indices);
}
