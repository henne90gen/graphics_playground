#pragma once

#include "scenes/Scene.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "opengl/Model.h"

#include <functional>
#include <memory>

class NormalMapping : public Scene {
public:
    explicit NormalMapping(SceneData data) : Scene(data, "NormalMapping") {};

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


    static void interleaveVertexData(const std::vector<glm::vec3> &tangents, const std::vector<glm::vec3> &biTangents,
                                     std::vector<float> &output);

    static void
    calculateTangentsAndBiTangents(const std::vector<glm::ivec3> &indices, const std::vector<glm::vec3> &vertices,
                                   const std::vector<glm::vec2> &uvs, std::vector<glm::vec3> &tangents,
                                   std::vector<glm::vec3> &biTangents);
};
