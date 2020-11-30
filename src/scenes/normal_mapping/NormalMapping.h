#pragma once

#include "Scene.h"
#include "opengl/Model.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
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
    std::unique_ptr<Model> model = {};
    glm::mat4 projectionMatrix;

    static void interleaveVertexData(const std::vector<glm::vec3> &tangents, const std::vector<glm::vec3> &biTangents,
                                     std::vector<float> &output);

    static void calculateTangentsAndBiTangents(const std::vector<glm::ivec3> &indices,
                                               const std::vector<glm::vec3> &vertices,
                                               const std::vector<glm::vec2> &uvs, std::vector<glm::vec3> &tangents,
                                               std::vector<glm::vec3> &biTangents);
    void renderMesh(const std::shared_ptr<OpenGLMesh> &mesh, const glm::vec3 &position, const glm::vec3 &rotation,
                    const float scale);
};
