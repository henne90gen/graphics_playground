#include "WebcamDemo.h"

void WebcamDemo::setup() {
    shader =
          std::make_shared<Shader>("scenes/webcam_demo/WebcamDemoVert.glsl", "scenes/webcam_demo/WebcamDemoFrag.glsl");
    shader->bind();
    onAspectRatioChange();

    va = std::make_shared<VertexArray>(shader);
    std::vector<glm::vec4> vertices = {
          {-1.0F, -1.0F, 0.0F, 0.0F},
          {1.0F, -1.0F, 1.0F, 0.0F},
          {1.0F, 1.0F, 1.0F, 1.0F},
          {-1.0F, 1.0F, 0.0F, 1.0F},
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

    glActiveTexture(GL_TEXTURE0);
    texture = std::make_shared<Texture>(GL_RED);
#if 0
    std::vector<glm::vec3> pixels = {
          {1.0, 0.0, 0.0}, //
          {0.0, 1.0, 0.0}, //
          {0.0, 0.0, 1.0}, //
          {1.0, 1.0, 1.0}, //
    };
    texture->update(pixels, 2, 2);
#else
    std::vector<glm::vec3> pixels = {{1.0, 0.0, 0.0}};
    texture->update(pixels, 1, 1);
#endif
    webcam = cv::VideoCapture(0);
    if (!webcam.isOpened()) {
        std::cout << "Could not open webcam!" << std::endl;
    }
}

void WebcamDemo::onAspectRatioChange() { projectionMatrix = glm::ortho(-1.0F, 1.0F, -1.0F, 1.0F); }

void WebcamDemo::tick() {
    cv::Size imageSize =
          cv::Size((int)webcam.get(cv::CAP_PROP_FRAME_WIDTH), (int)webcam.get(cv::CAP_PROP_FRAME_HEIGHT));
    cv::Mat buffer;
    std::cout << "Buffer type: " << buffer.depth() << " channels: " << buffer.channels() << std::endl;
    webcam >> buffer;
    texture->update(buffer.data, imageSize.width, imageSize.height);

    texture->bind();
    shader->bind();
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("textureSampler", 0);
    va->bind();

    GL_Call(glDrawElements(GL_TRIANGLES, va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    va->unbind();
    shader->unbind();
}
