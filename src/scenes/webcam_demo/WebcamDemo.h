#pragma once

#include "Scene.h"

#include <functional>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include "gl/Shader.h"
#include "gl/Texture.h"
#include "gl/VertexArray.h"

class WebcamDemo : public Scene {
  public:
    explicit WebcamDemo() : Scene("WebcamDemo"){};
    ~WebcamDemo() override = default;

    void setup() override;
    void tick() override;
    void destroy() override {}

    void onAspectRatioChange() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> va;
    std::shared_ptr<Texture> texture;
    glm::mat4 projectionMatrix;

    cv::VideoCapture webcam;
    cv::Size imageSize;
    cv::Mat imageBuffer;
};
