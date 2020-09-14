#include "WebcamDemo.h"

#include <opencv2/imgproc.hpp>

void WebcamDemo::setup() {
    shader =
          std::make_shared<Shader>("scenes/webcam_demo/WebcamDemoVert.glsl", "scenes/webcam_demo/WebcamDemoFrag.glsl");
    shader->bind();
    onAspectRatioChange();

    va = std::make_shared<VertexArray>(shader);
    std::vector<glm::vec4> vertices = {
          {-1.0F, -1.0F, 1.0F, 1.0F},
          {1.0F, -1.0F, 0.0F, 1.0F},
          {1.0F, 1.0F, 0.0F, 0.0F},
          {-1.0F, 1.0F, 1.0F, 0.0F},
    };
    BufferLayout layout = {
          {ShaderDataType::Float2, "position"},
          {ShaderDataType::Float2, "inUV"},
    };
    auto vb = std::make_shared<VertexBuffer>(vertices, layout);
    va->addVertexBuffer(vb);

    std::vector<glm::ivec3> indices = {
          {0, 1, 2},
          {0, 2, 3},
    };
    auto ib = std::make_shared<IndexBuffer>(indices);
    va->setIndexBuffer(ib);

    texture = std::make_shared<Texture>(GL_BGR, GL_RGB);
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

    std::vector<glm::vec3> pixels = {
          {1.0, 0.0, 0.0}, //
          {0.0, 1.0, 0.0}, //
          {0.0, 0.0, 1.0}, //
          {1.0, 1.0, 1.0}, //
    };
    texture->update(pixels, 2, 2);

    webcam = cv::VideoCapture(0);
    if (!webcam.isOpened()) {
        std::cout << "Could not open webcam!" << std::endl;
    }
}

void WebcamDemo::onAspectRatioChange() { projectionMatrix = glm::ortho(-1.0F, 1.0F, -1.0F, 1.0F); }

void WebcamDemo::tick() {
    static auto isGrayScale = true;
    cv::Size imageSize =
          cv::Size((int)webcam.get(cv::CAP_PROP_FRAME_WIDTH), (int)webcam.get(cv::CAP_PROP_FRAME_HEIGHT));
    cv::Mat buffer;
    webcam >> buffer;

    ImGui::Begin("Settings");
    ImGui::Text("Image Size: (%dx%d)", imageSize.width, imageSize.height);
    ImGui::Text("Channels: %d", buffer.channels());
    ImGui::Checkbox("Grayscale", &isGrayScale);
    ImGui::End();

    if (isGrayScale) {
        cv::Mat greyMat, colorMat;
        cv::cvtColor(buffer, greyMat, cv::COLOR_BGR2GRAY);
        buffer = greyMat;
        texture->setDataType(GL_RED);
    } else {
        texture->setDataType(GL_BGR);
    }

    texture->bind();
    texture->update(buffer.data, imageSize.width, imageSize.height);

    shader->bind();
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("textureSampler", 0);
    shader->setUniform("isGrayScale", isGrayScale);
    va->bind();

    GL_Call(glDrawElements(GL_TRIANGLES, va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    va->unbind();
    shader->unbind();
}
