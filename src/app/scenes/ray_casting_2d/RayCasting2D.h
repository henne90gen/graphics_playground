#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <memory>

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"

class RayCasting2D : public Scene {
public:
    RayCasting2D(SceneData &data)
            : Scene(data, "RayCasting2D") {};

    ~RayCasting2D() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;

};
