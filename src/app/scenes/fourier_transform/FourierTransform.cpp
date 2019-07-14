#include "FourierTransform.h"

#include "CodingTrain.h"

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
    static auto colors = createColors();
    static std::vector<glm::vec2> mousePositions = {};
    static std::vector<glm::vec2> drawnPoints = {};
    static float zoom = 0.5F;
    static float rotationAngle = 0.0F;
    static float rotationSpeed = 1.0F;
    static int fourierResolution = 200;
    static bool useMousePositions = true;
    int previousFourierResolution = fourierResolution;

    updateCoefficients(mousePositions, useMousePositions, fourierResolution);

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
    ImGui::DragFloat("Zoom", &zoom, 0.01F);
    ImGui::SliderInt("Fourier Resolution", &fourierResolution, 0, 500);
    ImGui::DragFloat("Rotation Speed", &rotationSpeed, 0.001F);
    ImGui::Checkbox("Use Mouse Positions", &useMousePositions);
    if (ImGui::Button("Reset Fourier") || fourierResolution != previousFourierResolution) {
        rotationAngle = 0.0F;
        drawnPoints.clear();
    }
    if (ImGui::Button("Reset Mouse Positions")) {
        mousePositions.clear();
        colors = createColors();
        drawnPoints.clear();
    }
    ImGui::End();

    rotationAngle += rotationSpeed / 100.0F;
    if (rotationAngle >= glm::two_pi<float>()) {
        rotationAngle = 0.0F;
        drawnPoints = {};
    }

    GL_Call(glDisable(GL_DEPTH_TEST));

    shader->bind();
    auto viewMatrix = glm::scale(glm::mat4(1.0F), glm::vec3(zoom));
    shader->setUniform("u_View", viewMatrix);

    drawCanvas(colors, mousePositions, viewMatrix);

    drawFourier(drawnPoints, rotationAngle);

    drawConnectedPoints(drawnPoints);
}

std::vector<glm::vec4> FourierTransform::createColors() {
    unsigned int width = getWidth();
    unsigned int height = getHeight();
    auto colors = std::vector<glm::vec4>(width * height);

    const glm::vec4 borderColor = glm::vec4(0.5, 0.5, 0.5, 1);
    for (unsigned int x = 0; x < width; x++) {
        colors[x] = borderColor;
        colors[(height - 2) * width + x] = borderColor;
    }
    for (unsigned int y = 0; y < height; y++) {
        colors[y * width] = borderColor;
        colors[y * width + width - 2] = borderColor;
    }

    return colors;
}

void FourierTransform::updateCoefficients(const std::vector<glm::vec2> &mousePositions, bool useMousePositions,
                                          int &fourierResolution) {
    auto x = std::vector<glm::vec2>();
    if (useMousePositions) {
        x = mousePositions;
    } else {
        const int skip = 8;
        for (unsigned long i = 0; i < dataPoints.size(); i += skip) {
            x.push_back(dataPoints[i]);
        }
    }
    int threshold = (int) (x.size() * 0.8);
    if (threshold > 500) {
        threshold = 500;
    }
    if (fourierResolution == 0 || fourierResolution >= threshold) {
        fourierResolution = threshold;
    }
    coefficients = Fourier::dft(x, fourierResolution);
}

void FourierTransform::drawFourier(std::vector<glm::vec2> &drawnPoints, float t) {
    shader->bind();
    shader->setUniform("u_RenderCanvas", false);
    fourierVertexArray->bind();

    std::vector<glm::vec2> vertices = {};
    glm::vec2 currentHead = {0, 0};
    vertices.emplace_back(0, 0);

    for (auto &coefficient : coefficients) {
        double angle = coefficient.frequency * t + coefficient.phase;
        currentHead.x += coefficient.amplitude * cos(angle);
        currentHead.y += coefficient.amplitude * sin(angle);
        vertices.push_back(currentHead);
    }

    drawnPoints.push_back(currentHead);

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

void FourierTransform::drawCanvas(std::vector<glm::vec4> &colors, std::vector<glm::vec2> &mousePositions,
                                  const glm::mat4 &viewMatrix) {
    shader->bind();
    shader->setUniform("u_RenderCanvas", true);
    vertexArray->bind();
    texture->bind();

    const unsigned int width = getWidth();
    const unsigned int height = getHeight();

    InputData *input = getInput();
    if (input->mouse.left) {
        auto &mousePos = input->mouse.pos;

        auto mouseDisplaySpace = glm::vec2(mousePos.x / width, (height - mousePos.y) / height);
        mouseDisplaySpace = mouseDisplaySpace * 2.0F - glm::vec2(1.0F, 1.0F);

        auto adjustedDisplayPos = glm::inverse(viewMatrix) * glm::vec4(mouseDisplaySpace, 0.0, 0.0);
        auto canvasPos = (glm::vec2(adjustedDisplayPos.x, adjustedDisplayPos.y) + glm::vec2(1.0F, 1.0F)) / 2.0F;
        canvasPos = glm::vec2(canvasPos.x * width, height - (canvasPos.y * height));

        if ((canvasPos.x >= 0 && canvasPos.x < width) && (canvasPos.y >= 0 && canvasPos.y < height)) {
            unsigned int i =
                    (height - (unsigned int) canvasPos.y) * width + (unsigned int) canvasPos.x;
            colors[i] = {1.0, 1.0, 1.0, 1.0};
            mousePositions.emplace_back(adjustedDisplayPos.x, adjustedDisplayPos.y);
        }
    }
    texture->update(colors.data(), width, height);

    GL_Call(glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}
