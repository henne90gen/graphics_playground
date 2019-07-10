#include "FourierTransform.h"

#include "fourier_transform/Fourier.h"

void FourierTransform::setup() {
    shader = std::make_shared<Shader>("../../../src/app/scenes/fourier_transform/FourierTransformVert.glsl",
                                      "../../../src/app/scenes/fourier_transform/FourierTransformFrag.glsl");
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();

    std::vector<glm::vec2> vertices = {
            {-1.0, -1.0},
            {0.0,  0.0},
            {1.0,  -1.0},
            {1.0,  0.0},
            {1.0,  1.0},
            {1.0,  1.0},
            {-1.0, 1.0},
            {0.0,  1.0}
    };
    BufferLayout layout = {
            {ShaderDataType::Float2, "a_Position"},
            {ShaderDataType::Float2, "a_UV"}
    };
    auto vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
    vertexArray->addVertexBuffer(vertexBuffer);

    std::vector<glm::ivec3> indices = {
            {0, 1, 2},
            {0, 2, 3}
    };
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    vertexArray->setIndexBuffer(indexBuffer);

    texture = std::make_shared<Texture>(GL_RGBA);
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform<int>("u_TextureSampler", 0);

    fourierVertexArray = std::make_shared<VertexArray>(shader);
    fourierVertexArray->bind();
}

void FourierTransform::destroy() {
    shader.reset();
}

void FourierTransform::tick() {
    static std::vector<glm::vec2> mousePositions = {};
    static std::vector<glm::vec2> drawnPoints = {};
    static float zoom = 0.5F;
    static float rotationAngle = 0.0F;
    static float animationSpeed = 0.01F;
    static int drawResolution = 200;
    auto coefficients = Fourier::calculate(mousePositions, 0);

    ImGui::Begin("Settings");
    ImGui::Text("Mouse: (%f, %f)", getInput()->mouse.pos.x, getInput()->mouse.pos.y);
    float mouseX = getInput()->mouse.pos.x / getWidth();
    mouseX = mouseX * 2.0F - 1.0F;
    float mouseY = (getHeight() - getInput()->mouse.pos.y) / getHeight();
    mouseY = mouseY * 2.0F - 1.0F;
    ImGui::Text("Relative Mouse: (%f, %f)", mouseX, mouseY);
    ImGui::Text("Num of Coefficients: %zu", coefficients.size());
    ImGui::Text("Num of Mouse Positions: %zu", mousePositions.size());
    ImGui::Text("Num of Drawn Points: %zu", drawnPoints.size());
    ImGui::Text("Rotation Angle: %f", rotationAngle);
    ImGui::DragFloat("Animation speed", &animationSpeed, 0.001F);
    ImGui::DragFloat("Zoom", &zoom, 0.01F);
    ImGui::DragInt("Draw Resolution", &drawResolution);
    ImGui::End();

    rotationAngle += animationSpeed;
    if (rotationAngle >= glm::two_pi<float>()) {
        rotationAngle = 0.0F;
        drawnPoints = {};
    }

    GL_Call(glDisable(GL_DEPTH_TEST));

    shader->bind();
    auto viewMatrix = glm::scale(glm::mat4(1.0F), glm::vec3(zoom));
    shader->setUniform("u_View", viewMatrix);

//    drawCanvas(mousePositions);

    coefficients = {
            {0.0,  0.5},
            {0.5,  0.0},
            {0.0,  -0.5},
            {0.25, 0.0},
            {0.0,  -0.25},
            {0.1,  0.0},
            {0.0,  -0.1},
            {0.05, 0.0},
            {0.0,  -0.05},
    };
    drawFourier(coefficients, drawnPoints, rotationAngle, drawResolution);

    drawConnectedPoints(drawnPoints);
}

void FourierTransform::drawFourier(const std::vector<glm::vec2> &coefficients, std::vector<glm::vec2> &drawnPoints,
                                   float t, unsigned int drawResolution) {
    shader->bind();
    shader->setUniform("u_RenderCanvas", false);
    fourierVertexArray->bind();

    std::vector<glm::vec2> vertices = {};
    glm::vec2 currentHead = {0, 0};
    vertices.emplace_back(0, 0);
    for (unsigned long i = 0; i < coefficients.size(); i++) {
        auto coefficient = coefficients[i];
        glm::mat4 rotation = glm::rotate(glm::identity<glm::mat4>(), (float) i * t, {0.0, 0.0, 1.0});
        auto rotatedCoefficient = rotation * glm::vec4(coefficient, 0.0, 1.0);
        currentHead += glm::vec2(rotatedCoefficient.x, rotatedCoefficient.y);
        vertices.push_back(currentHead);
    }

    if (glm::degrees(t) / 360.0 * (float) drawResolution >= drawnPoints.size()) {
        drawnPoints.push_back(currentHead);
    }

    BufferLayout layout = {
            {ShaderDataType::Float2, "a_Position"}
    };
    auto vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
    fourierVertexArray->addVertexBuffer(vertexBuffer);

    std::vector<unsigned int> indices = {};
    for (unsigned long i = 0; i < coefficients.size(); i++) {
        indices.push_back(i);
        indices.push_back(i + 1);
    }
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    fourierVertexArray->setIndexBuffer(indexBuffer);

    GL_Call(glDrawElements(GL_LINES, fourierVertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void FourierTransform::drawConnectedPoints(const std::vector<glm::vec2> &drawnPoints) {
    shader->bind();
    shader->setUniform("u_RenderCanvas", false);
    fourierVertexArray->bind();

    BufferLayout layout = {
            {ShaderDataType::Float2, "a_Position"}
    };
    auto vertexBuffer = std::make_shared<VertexBuffer>(drawnPoints, layout);
    fourierVertexArray->addVertexBuffer(vertexBuffer);

    std::vector<unsigned int> indices = {};
    for (unsigned long i = 0; i < drawnPoints.size() - 1; i++) {
        indices.push_back(i);
        indices.push_back(i + 1);
    }
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    fourierVertexArray->setIndexBuffer(indexBuffer);

    GL_Call(glDrawElements(GL_LINES, fourierVertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void FourierTransform::drawCanvas(std::vector<glm::vec2> &mousePositions) {
    shader->bind();
    shader->setUniform("u_RenderCanvas", true);
    vertexArray->bind();
    texture->bind();

    const unsigned int width = getWidth();
    const unsigned int height = getHeight();
    static auto colors = std::vector<glm::vec4>(width * height);
    unsigned int i = (height - (unsigned int) getInput()->mouse.pos.y) * width + (unsigned int) getInput()->mouse.pos.x;
    colors[i] = {1.0, 1.0, 1.0, 1.0};
    mousePositions.emplace_back(getInput()->mouse.pos.x, getInput()->mouse.pos.y);
    texture->update(colors.data(), width, height);

    GL_Call(glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}
