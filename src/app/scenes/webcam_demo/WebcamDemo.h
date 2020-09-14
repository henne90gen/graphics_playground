#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "opengl/Texture.h"

class WebcamDemo : public Scene {
  public:
    explicit WebcamDemo(SceneData &data) : Scene(data, "WebcamDemo"){};
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
