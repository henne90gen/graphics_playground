#include "LSystems.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 10000.0F;

DEFINE_SHADER(l_systems_LSystems)

void LSystems::setup() {
    shader = SHADER(l_systems_LSystems);

    va = std::make_shared<VertexArray>(shader);

    std::vector<float> vertices = {
          -1.0, -1.0, 0.0, //
          1.0,  -1.0, 0.0, //
          1.0,  -1.0, 0.0, //
          1.0,  1.0,  0.0, //
          1.0,  1.0,  0.0, //
          -1.0, 1.0,  0.0, //
          -1.0, 1.0,  0.0, //
          -1.0, -1.0, 0.0, //
    };
    BufferLayout bufferLayout = {{ShaderDataType::Float3, "position"}};
    vb = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    va->addVertexBuffer(vb);
}

void LSystems::destroy() {}

void LSystems::tick() {
    static auto cameraPosition = glm::vec3(0.0F, 0.0F, -100.0F);
    static auto cameraRotation = glm::vec3();
    static auto numIterations = 3;
    static auto timerSeconds = 1.0;
    static auto vertexCount = 0;

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition), 0.01F);
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), 0.01F);
    ImGui::Separator();
    ImGui::SliderInt("Number of Iterations", &numIterations, 0, 20);
    ImGui::End();

    shader->bind();
    va->bind();

    const auto projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    shader->setUniform("u_Projection", projectionMatrix);
    const auto viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    shader->setUniform("u_View", viewMatrix);
    const auto modelMatrix = glm::mat4(1.0);
    shader->setUniform("u_Model", modelMatrix);

    timerSeconds += getLastFrameTime();
    if (timerSeconds >= 0.5) {
        timerSeconds = 0.0;
        numIterations++;
        if (numIterations > 20) {
            numIterations = 5;
        }
        std::vector<glm::vec3> vertices = {};

        dragonCurve(vertices, numIterations);

        vb->update(vertices);
        vertexCount = vertices.size();
    }

    GL_Call(glDrawArrays(GL_LINES, 0, vertexCount));

    va->unbind();
    shader->unbind();
}

void line(std::vector<glm::vec3> &vertices, float x1, float y1, float x2, float y2) {
    vertices.emplace_back(x1, y1, 0.0);
    vertices.emplace_back(x2, y2, 0.0);
}

void simulateLSystem(const unsigned int numIterations, const std::string &startSequence,
                     const std::function<std::string(char)> &sequenceUpdateFunc,
                     const std::function<void(char)> &turtleFunc) {
    std::string sequence = startSequence;
    for (unsigned int i = 0; i < numIterations; i++) {
        std::string newSequence;
        for (char c : sequence) {
            newSequence += sequenceUpdateFunc(c);
        }
        sequence = newSequence;
    }

    for (char c : sequence) {
        turtleFunc(c);
    }
}

void dragonCurve(std::vector<glm::vec3> &vertices, const unsigned int numIterations) {
    const float d = 0.05F;
    const float angleDelta = glm::pi<float>() / 2.0F;
    float x = 0.0F;
    float y = 0.0F;
    float angle = 0.0F;
    simulateLSystem(
          numIterations, "FX",
          [](char c) -> std::string {
              if (c == 'X') {
                  return "X+YF+";
              }
              if (c == 'Y') {
                  return "-FX-Y";
              }
              return std::string(1, c);
          },
          [&vertices, &d, &angleDelta, &x, &y, &angle](char c) {
              if (c == 'F') {
                  const float nextX = x + d * std::cos(angle);
                  const float nextY = y + d * std::sin(angle);
                  line(vertices, x, y, nextX, nextY);
                  x = nextX;
                  y = nextY;
              } else if (c == '-') {
                  angle -= angleDelta;
              } else if (c == '+') {
                  angle += angleDelta;
              }
          });
}
