#include "MarchingCubesScene.h"

#include <array>
#include <glad/glad.h>
#include <glm/ext.hpp>

#include "Main.h"
#include "util/OpenGLUtils.h"

DEFINE_SCENE_MAIN(MarchingCubesScene)
DEFINE_DEFAULT_SHADERS(marching_cubes_MarchingCubes)

void MarchingCubesScene::setup() {
    shader = CREATE_DEFAULT_SHADER(marching_cubes_MarchingCubes);
    shader->bind();

    cubeVertexArray = std::make_shared<VertexArray>(shader);
    cubeVertexArray->bind();

    BufferLayout bufferLayout = {
          {ShaderDataType::Float3, "a_Position"},
    };
    auto cubeCornersVec = std::vector<glm::vec3>(cubeCorners.begin(), cubeCorners.end());
    auto positionBuffer = std::make_shared<VertexBuffer>(cubeCornersVec, bufferLayout);
    cubeVertexArray->addVertexBuffer(positionBuffer);

    std::vector<unsigned int> indices = {
          // bottom
          4, 5, 1, 0, 4, // NOLINT(cppcoreguidelines-avoid-magic-numbers)

          // top
          7, 6, 2, 3, 7, // NOLINT(cppcoreguidelines-avoid-magic-numbers)

          // edges
          6, 5, 1, 2, 3, 0 // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    };
    cubeIndexBuffer = std::make_shared<IndexBuffer>(indices);
    cubeIndexBuffer->bind();

    marchingCubes = std::make_shared<MarchingCubes>();

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

void MarchingCubesScene::destroy() {}

void MarchingCubesScene::tick() {
    static auto modelRotation = glm::vec3();
    static bool rotate = true;
    static bool drawWireframe = false;
    static float rotationSpeed = 0.03F;

    if (rotate) {
        modelRotation.y += rotationSpeed;
    }

    showSettings(rotationSpeed, rotate, drawWireframe);

    {
        RECORD_SCOPE_NAME("MarchingCubes");
        marchingCubes->step();
    }

    {
        RECORD_SCOPE_NAME("Render");
        glm::vec3 dimensions =
              glm::vec3(static_cast<float>(marchingCubes->width), static_cast<float>(marchingCubes->height),
                        static_cast<float>(marchingCubes->depth));
        const float halfScale = -0.5F;
        glm::vec3 modelCenter = dimensions * halfScale;

        shader->bind();

        glm::mat4 modelMatrix = glm::mat4(1.0F);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2F));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
        modelMatrix = glm::translate(modelMatrix, modelCenter);
        shader->setUniform("u_Model", modelMatrix);
        shader->setUniform("u_View", getCamera().getViewMatrix());
        shader->setUniform("u_Projection", getCamera().getProjectionMatrix());
        shader->setUniform("u_Dimensions", dimensions);

        drawSurface(drawWireframe);
        drawCube();

        shader->unbind();
    }
}

void MarchingCubesScene::showSettings(float &rotationSpeed, bool &rotate, bool &drawWireframe) const {
    ImGui::Begin("Settings");

    ImGui::Checkbox("Rotate", &rotate);
    ImGui::Checkbox("Wireframe", &drawWireframe);
    ImGui::Checkbox("Interpolate", &marchingCubes->interpolate);
    ImGui::Checkbox("Animate", &marchingCubes->animate);

    ImGui::DragFloat("Rotation Speed", &rotationSpeed, 0.001F);
    ImGui::DragInt("Animation Speed", &marchingCubes->animationSpeed, 1.0F, 1, 100);

    std::array<int, 3> dimensions = {static_cast<int>(marchingCubes->width), static_cast<int>(marchingCubes->height),
                                     static_cast<int>(marchingCubes->depth)};
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    ImGui::DragInt3("Dimensions", dimensions.data(), 1.0F, 1, 100);
    marchingCubes->width = dimensions[0];
    marchingCubes->height = dimensions[1];
    marchingCubes->depth = dimensions[2];

    ImGui::SliderFloat("Surface Level", &marchingCubes->surfaceLevel, 0.0F, 1.0F);
    ImGui::SliderFloat("Frequency", &marchingCubes->frequency, 0.0F, 1.0F);

    ImGui::NoiseTypeSelector(&marchingCubes->noiseType);

    if (ImGui::Button("Start")) {
        marchingCubes->start();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        marchingCubes->reset();
    }

    ImGui::Text("Number of vertices: %zu", marchingCubes->getVertices().size());
    ImGui::Text("Number of indices: %zu", marchingCubes->getIndices().size());

    ImGui::End();
}

void MarchingCubesScene::drawCube() const {
    shader->setUniform("u_Offset", marchingCubes->getCubeTranslation());
    cubeVertexArray->bind();
    GL_Call(glDrawElements(GL_LINE_LOOP, cubeIndexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));
    cubeVertexArray->unbind();
}

void MarchingCubesScene::drawSurface(bool drawWireframe) const {
    surfaceVertexArray->bind();

    surfaceVertexBuffer->update(marchingCubes->getVertices());
    surfaceIndexBuffer->update(marchingCubes->getIndices());

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
