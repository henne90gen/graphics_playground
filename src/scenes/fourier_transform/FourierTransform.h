#pragma once

#include "Scene.h"
#include "opengl/IndexBuffer.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"

#include <functional>
#include <memory>

#include "fourier_transform/Fourier.h"

class FourierTransform : public Scene {
  public:
    explicit FourierTransform() : Scene("FourierTransform"){};

    ~FourierTransform() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<VertexArray> fourierVertexArray;
    std::shared_ptr<Texture> texture;
    unsigned int canvasWidth = 512;
    unsigned int canvasHeight = 512;

    void drawCanvas(std::vector<glm::vec4> &colors, std::vector<glm::vec2> &mousePositions,
                    const glm::mat4 &viewMatrix);

    void drawFourier(std::vector<glm::vec2> &drawnPoints, float t);

    void drawConnectedPoints(const std::vector<glm::vec2> &drawnPoints);

    std::vector<fourier::DataPoint> coefficients;

    void updateCoefficients(const std::vector<glm::vec2> &mousePositions, bool useMousePositions,
                            int &fourierResolution);

    std::vector<glm::vec4, std::allocator<glm::vec4>> createColors() const;
};
