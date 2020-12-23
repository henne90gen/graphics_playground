#include "LSystems.h"

#include "Main.h"
#include "util/ImGuiUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 10000.0F;

DEFINE_SCENE_MAIN(LSystems)
DEFINE_DEFAULT_SHADER(l_systems_LSystems)

void LSystems::setup() {
    shader = CREATE_DEFAULT_SHADER(l_systems_LSystems);

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
    const std::array<std::function<void(std::vector<glm::vec3> &, unsigned int)>, 4> lSystems = {
          dragonCurve,        //
          fractalPlant,       //
          sierpinskiTriangle, //
          binaryTree,         //
    };
    const std::array<const char *, 4> lSystemNames = {
          "Dragon Curve",        //
          "Fractal Plant",       //
          "Sierpinski Triangle", //
          "Binary Tree",         //
    };
    static auto cameraPosition = glm::vec3(0.0F, 0.0F, -100.0F);
    static auto cameraRotation = glm::vec3();
    static auto numIterations = 3;
    static auto vertexCount = 0;
    static unsigned int currentLSystem = 0;
    auto previousNumIterations = numIterations;
    auto previousLSystem = currentLSystem;

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition), 0.01F);
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), 0.01F);
    ImGui::Separator();
    ImGui::SliderInt("Number of Iterations", &numIterations, 0, 20);
    ImGui::Combo("Noise Algorithm", reinterpret_cast<int *>(&currentLSystem), lSystemNames.data(), lSystemNames.size());
    ImGui::End();

    shader->bind();
    va->bind();

    const auto projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    shader->setUniform("u_Projection", projectionMatrix);
    const auto viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    shader->setUniform("u_View", viewMatrix);
    const auto modelMatrix = glm::mat4(1.0);
    shader->setUniform("u_Model", modelMatrix);

    if (numIterations != previousNumIterations || currentLSystem != previousLSystem || vertexCount == 0) {
        std::vector<glm::vec3> vertices = {};

        lSystems[currentLSystem](vertices, numIterations);

        vb->update(vertices);
        vertexCount = vertices.size();
    }

    GL_Call(glDrawArrays(GL_LINES, 0, vertexCount));

    va->unbind();
    shader->unbind();
}

inline void line(std::vector<glm::vec3> &vertices, float x1, float y1, float x2, float y2) {
    vertices.emplace_back(x1, y1, 0.0);
    vertices.emplace_back(x2, y2, 0.0);
}

void simulateLSystem(std::vector<glm::vec3> &vertices, const unsigned int numIterations, const LSystem &l) {
    std::string sequence = l.startSequence;
    for (unsigned int i = 0; i < numIterations; i++) {
        std::string newSequence;
        for (char c : sequence) {
            auto result = l.replacements.find(c);
            if (result != l.replacements.end()) {
                newSequence += result->second;
            } else {
                newSequence += c;
            }
        }
        sequence = newSequence;
    }

    struct StackElement {
        StackElement(float x, float y, float angle) : x(x), y(y), angle(angle){};
        float x;
        float y;
        float angle;
    };
    std::vector<StackElement> stack = {};
    float x = 0.0F;
    float y = 0.0F;
    float angle = 0.0F;
    for (char c : sequence) {
        if (c == 'F' || c == 'G') {
            const float nextX = x + l.d * std::cos(angle);
            const float nextY = y + l.d * std::sin(angle);
            line(vertices, x, y, nextX, nextY);
            x = nextX;
            y = nextY;
        } else if (c == '[') {
            stack.emplace_back(x, y, angle);
        } else if (c == ']') {
            auto &elem = stack.back();
            x = elem.x;
            y = elem.y;
            angle = elem.angle;
            stack.pop_back();
        } else if (c == '+') {
            angle -= l.angleDelta;
        } else if (c == '-') {
            angle += l.angleDelta;
        }
    }
}

void dragonCurve(std::vector<glm::vec3> &vertices, const unsigned int numIterations) {
    const float d = 0.5F;
    const float angleDelta = glm::pi<float>() / -2.0F;
    std::unordered_map<char, std::string> replacements = {};
    replacements.emplace('X', std::string("X+YF+"));
    replacements.emplace('Y', std::string("-FX-Y"));
    LSystem l = {"FX", replacements, d, angleDelta};
    simulateLSystem(vertices, numIterations, l);
}

void fractalPlant(std::vector<glm::vec3> &vertices, const unsigned int numIterations) {
    const float d = 0.05;
    const float angleDelta = glm::pi<float>() / 7.2F;
    std::unordered_map<char, std::string> replacements = {};
    replacements.emplace('X', std::string("F+[[X]-X]-F[-FX]+X"));
    replacements.emplace('F', std::string("FF"));
    LSystem l = {"X", replacements, d, angleDelta};
    simulateLSystem(vertices, numIterations, l);
}

void sierpinskiTriangle(std::vector<glm::vec3> &vertices, const unsigned int numIterations) {
    const float d = 0.05;
    const float angleDelta = glm::pi<float>() * 2.0F / 3.0F;
    std::unordered_map<char, std::string> replacements = {};
    replacements.emplace('F', std::string("F-G+F+G-F"));
    replacements.emplace('G', std::string("GG"));
    LSystem l = {"F-G-G", replacements, d, angleDelta};
    simulateLSystem(vertices, numIterations, l);
}

void binaryTree(std::vector<glm::vec3> &vertices, const unsigned int numIterations) {
    const float d = 0.05;
    const float angleDelta = glm::pi<float>() / 4.0F;
    std::unordered_map<char, std::string> replacements = {};
    replacements.emplace('F', std::string("FF"));
    replacements.emplace('G', std::string("F[-G]+G"));
    LSystem l = {"G", replacements, d, angleDelta};
    simulateLSystem(vertices, numIterations, l);
}
