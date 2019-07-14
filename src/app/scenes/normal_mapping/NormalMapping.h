#pragma once

#include "scenes/Scene.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"

#include <functional>
#include <memory>

class NormalMapping : public Scene {
public:
    NormalMapping(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "NormalMapping") {};

    ~NormalMapping() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<Texture> normalMap;

    void interleaveVertexData(const std::vector<glm::vec3> &positions, const std::vector<glm::vec2> &uvs,
                              const std::vector<glm::vec3> &normals, std::vector<float> &output);

};
