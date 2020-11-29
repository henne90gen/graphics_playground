#pragma once

#include "scenes/Scene.h"

#include <functional>

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"

class LSystems : public Scene {
  public:
    explicit LSystems(SceneData &data) : Scene(data, "LSystems"){};
    ~LSystems() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> va;
    std::shared_ptr<VertexBuffer> vb;
};

struct LSystem {
    std::string startSequence = "";
    std::unordered_map<char, std::string> replacements = {};
    float d = 0.05F;
    float angleDelta = glm::pi<float>();
};

void line(std::vector<glm::vec3> &vertices, float x1, float y1, float x2, float y2);
void simulateLSystem(unsigned int numIterations, const LSystem &l);

void dragonCurve(std::vector<glm::vec3> &vertices, unsigned int numIterations);
void fractalPlant(std::vector<glm::vec3> &vertices, unsigned int numIterations);
void sierpinskiTriangle(std::vector<glm::vec3> &vertices, unsigned int numIterations);
void binaryTree(std::vector<glm::vec3> &vertices, const unsigned int numIterations);
