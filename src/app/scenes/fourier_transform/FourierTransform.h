#pragma once

#include "scenes/Scene.h"
#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/IndexBuffer.h"
#include "opengl/Texture.h"

#include <functional>
#include <memory>

#include "fourier_transform/Fourier.h"

class FourierTransform : public Scene {
public:
    FourierTransform(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "FourierTransform") {};

    ~FourierTransform() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<VertexArray> fourierVertexArray;
    std::shared_ptr<Texture> texture;

    void
    drawCanvas(std::vector<glm::vec4> &colors, std::vector<glm::vec2> &mousePositions, const glm::mat4 &viewMatrix);

    void drawFourier(const std::vector<fourier_result> &coefficients, std::vector<glm::vec2> &drawnPoints, float t,
                     unsigned int drawResolution);

    void drawConnectedPoints(const std::vector<glm::vec2> &drawnPoints);

    std::vector<fourier_result> coefficients;
};
