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

void line(std::vector<glm::vec3> &vertices, float x1, float y1, float x2, float y2);
void simulateLSystem(unsigned int numIterations, const std::string &startSequence,
                     const std::function<std::string(char)> &sequenceUpdateFunc,
                     const std::function<void(char)> &turtleFunc);
void dragonCurve(std::vector<glm::vec3> &vertices, unsigned int numIterations);
