#pragma once

#include "Model.h"
#include "Scene.h"
#include "gl/Shader.h"
#include "gl/Texture.h"
#include "gl/VertexArray.h"
#include "util/TimeUtils.h"

#include <functional>
#include <memory>

class NormalMapping : public Scene {
  public:
    explicit NormalMapping() : Scene("NormalMapping"){};

    ~NormalMapping() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  protected:
    void onAspectRatioChange() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<Texture> normalMap;
    Model model = {};
    glm::mat4 projectionMatrix;

    static void interleaveVertexData(const std::vector<glm::vec3> &tangents, const std::vector<glm::vec3> &biTangents,
                                     std::vector<float> &output);

    static void calculateTangentsAndBiTangents(const std::vector<glm::ivec3> &indices,
                                               const std::vector<glm::vec3> &vertices,
                                               const std::vector<glm::vec2> &uvs, std::vector<glm::vec3> &tangents,
                                               std::vector<glm::vec3> &biTangents);
    void renderMesh(const OpenGLMesh &mesh, const glm::vec3 &rotation, const float scale);
};
