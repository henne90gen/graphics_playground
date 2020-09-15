#include "WebcamDemo.h"

#include <opencv2/imgproc.hpp>

SHADER_DEFINITION(webcam_demo_WebcamDemoVert)
SHADER_DEFINITION(webcam_demo_WebcamDemoFrag)

void WebcamDemo::setup() {
    shader = SHADER(webcam_demo_WebcamDemoVert, webcam_demo_WebcamDemoFrag);

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

    imageSize = cv::Size((int)webcam.get(cv::CAP_PROP_FRAME_WIDTH), (int)webcam.get(cv::CAP_PROP_FRAME_HEIGHT));
    imageBuffer = cv::Mat(imageSize.width, imageSize.height, CV_8UC3);
}

void WebcamDemo::onAspectRatioChange() { projectionMatrix = glm::ortho(-1.0F, 1.0F, -1.0F, 1.0F); }

void WebcamDemo::tick() {
    static auto isGrayScale = true;
    static int updateEveryXFrames = 5;

    ImGui::Begin("Settings");
    ImGui::Text("Image Size: (%dx%d)", imageSize.width, imageSize.height);
    ImGui::Text("Channels: %d", imageBuffer.channels());
    ImGui::Checkbox("Grayscale", &isGrayScale);
    ImGui::DragInt("Update every X frames", &updateEveryXFrames, 1.0F, 1, 100);
    ImGui::End();

    texture->bind();

    static auto counter = 0;
    counter++;
    if (counter % updateEveryXFrames == 0) {
        webcam >> imageBuffer;
        texture->update(imageBuffer.data, imageSize.width, imageSize.height);
    }

    shader->bind();
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("textureSampler", 0);
    shader->setUniform("isGrayScale", isGrayScale);
    va->bind();

    GL_Call(glDrawElements(GL_TRIANGLES, va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    va->unbind();
    shader->unbind();
}
