#include "MetaBallsScene.h"

#include "Main.h"
#include "util/ImGuiUtils.h"
#include "util/RenderUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

DEFINE_SCENE_MAIN(MetaBallsScene)
DEFINE_DEFAULT_SHADERS(meta_balls_MetaBalls)

struct AnimatedBall {
    glm::vec3 startPos;
    glm::vec3 endPos;
    float t = 0.0F;
    float animationDir = 1.0F;
    glm::vec3 position = glm::vec3();
};

void updateAnimatedBall(AnimatedBall &ball, float delta, float speed);

void MetaBallsScene::setup() {
    getCamera().setDistance(50);
    shader = CREATE_DEFAULT_SHADER(meta_balls_MetaBalls);
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
    static AnimatedBall ball1 = {glm::vec3(20.0F, 10.0F, 10.0F), glm::vec3(20.0F, 30.0F, 10.0F)};
    static AnimatedBall ball2 = {glm::vec3(10.0F, 20.0F, 10.0F), glm::vec3(35.0F, 20.0F, 10.0F)};

    // TODO(henne): add unmodified spheres for metaballs

    static float animationSpeed = 0.3F;
    auto timeDelta = static_cast<float>(getLastFrameTime());
    updateAnimatedBall(ball1, timeDelta, animationSpeed);
    updateAnimatedBall(ball2, timeDelta, 2.0F * animationSpeed);

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
        ImGui::Separator();
        ImGui::Text("X: %f | Y: %f | Z: %f", ball1.position.x, ball1.position.y, ball1.position.z);
        ImGui::Separator();
        ImGui::Text("X: %f | Y: %f | Z: %f", ball2.position.x, ball2.position.y, ball2.position.z);
        ImGui::Separator();
        ImGui::Text("Time delta: %f", timeDelta);
        ImGui::End();
    }

    {
        RECORD_SCOPE_NAME("Update");
        const float radiusSq = radius * radius;
        std::vector<MetaBall> metaballs = {};
        metaballs.push_back({positionBigBall, radiusSq});
        metaballs.push_back({ball1.position, radiusSq});
        metaballs.push_back({ball2.position, radiusSq / 2.0F});

        updateSurface(dimensions, funcType, metaballs);
    }

    const float halfScale = -0.5F;
    glm::vec3 modelCenter = glm::vec3(dimensions) * halfScale;
    glm::mat4 viewMatrix = getCamera().getViewMatrix();
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

void updateAnimatedBall(AnimatedBall &ball, float delta, float speed) {
    ball.t += speed * ball.animationDir * delta;
    const auto dir = ball.endPos - ball.startPos;
    ball.position = ball.startPos + ball.t * dir;
    if (ball.t < 0.0F || ball.t > 1.0F) {
        ball.animationDir *= -1.0F;
    }
}
