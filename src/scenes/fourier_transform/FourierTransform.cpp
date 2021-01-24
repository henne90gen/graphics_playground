#include "FourierTransform.h"

#include <algorithm>

#include "CodingTrain.h"
#include "Main.h"

const int MAX_FOURIER_RESOLUTION = 500;

DEFINE_SCENE_MAIN(FourierTransform)
DEFINE_DEFAULT_SHADERS(fourier_transform_FourierTransform)

void FourierTransform::setup() {
    shader = CREATE_DEFAULT_SHADER(fourier_transform_FourierTransform);
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();

    std::vector<glm::vec2> vertices = {{-1.0, -1.0}, {0.0, 0.0}, {1.0, -1.0}, {1.0, 0.0},
                                       {1.0, 1.0},   {1.0, 1.0}, {-1.0, 1.0}, {0.0, 1.0}};
    BufferLayout layout = {{ShaderDataType::Vec2, "a_Position"}, {ShaderDataType::Vec2, "a_UV"}};
    auto vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
    vertexArray->addVertexBuffer(vertexBuffer);

    std::vector<glm::ivec3> indices = {{0, 1, 2}, {0, 2, 3}};
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    vertexArray->setIndexBuffer(indexBuffer);

    TextureSettings settings = {};
    settings.dataType = GL_RGBA;
    texture = std::make_shared<Texture>(settings);
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform("u_TextureSampler", 0);

    fourierVertexArray = std::make_shared<VertexArray>(shader);
    fourierVertexArray->bind();

    GL_Call(glDisable(GL_DEPTH_TEST));
}

void FourierTransform::destroy() {
    shader.reset();
    GL_Call(glEnable(GL_DEPTH_TEST));
}

void FourierTransform::tick() {
    static auto colors = createColors();
    static std::vector<glm::vec2> mousePositions = {};
    static std::vector<glm::vec2> drawnPoints = {};
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static float zoom = 0.5F;
    static float rotationAngle = 0.0F;
    static float rotationSpeed = 1.0F;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    static int fourierResolution = 200;
    static bool useMousePositions = true;
    int previousFourierResolution = fourierResolution;

    updateCoefficients(mousePositions, useMousePositions, fourierResolution);

    ImGui::Begin("Settings");
    ImGui::Text("Num of Coefficients: %zu", coefficients.size());
    ImGui::Text("Num of Mouse Positions: %zu", mousePositions.size());
    ImGui::Text("Num of Drawn Points: %zu", drawnPoints.size());
    ImGui::Text("Rotation Angle: %f", rotationAngle);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    ImGui::DragFloat("Zoom", &zoom, 0.01F);
    ImGui::SliderInt("Fourier Resolution", &fourierResolution, 0, MAX_FOURIER_RESOLUTION);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
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

    shader->bind();
    auto viewMatrix = glm::scale(glm::mat4(1.0F), glm::vec3(zoom));
    shader->setUniform("u_View", viewMatrix);

    drawCanvas(colors, mousePositions, viewMatrix);

    drawFourier(drawnPoints, rotationAngle);

    drawConnectedPoints(drawnPoints);
}

std::vector<glm::vec4> FourierTransform::createColors() const {
    auto colors = std::vector<glm::vec4>(canvasWidth * canvasHeight);

    const glm::vec4 borderColor = glm::vec4(0.5, 0.5, 0.5, 1);
    for (unsigned int x = 0; x < canvasWidth; x++) {
        colors[x] = borderColor;
        colors[(canvasHeight - 2) * canvasWidth + x] = borderColor;
    }
    for (unsigned int y = 0; y < canvasHeight; y++) {
        colors[y * canvasWidth] = borderColor;
        colors[y * canvasWidth + canvasWidth - 2] = borderColor;
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
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    int threshold = static_cast<int>(x.size() * 0.8);
    if (threshold > MAX_FOURIER_RESOLUTION) {
        threshold = MAX_FOURIER_RESOLUTION;
    }
    if (fourierResolution == 0 || fourierResolution >= threshold) {
        fourierResolution = threshold;
    }
    coefficients = fourier::dft2(x, fourierResolution);
    std::sort(coefficients.begin(), coefficients.end(),
              [](fourier::DataPoint &a, fourier::DataPoint &b) { return a.magnitude > b.magnitude; });
}

void FourierTransform::drawFourier(std::vector<glm::vec2> &drawnPoints, float t) {
    shader->bind();
    shader->setUniform("u_RenderCanvas", false);
    fourierVertexArray->bind();

    std::vector<glm::vec2> vertices = {};
    glm::vec2 currentHead = {0, 0};
    vertices.emplace_back(0, 0);

    for (auto &coefficient : coefficients) {
        double angle = static_cast<double>(coefficient.frequency) * t + coefficient.phase;
        currentHead.x += coefficient.magnitude * cos(angle);
        currentHead.y += coefficient.magnitude * sin(angle);
        vertices.push_back(currentHead);
    }

    drawnPoints.push_back(currentHead);

    BufferLayout layout = {{ShaderDataType::Vec2, "a_Position"}};
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

    BufferLayout layout = {{ShaderDataType::Vec2, "a_Position"}};
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

    const InputData &input = getInput();
    if (input.mouse.left) {
        auto &mousePos = input.mouse.pos;
        auto mappedMousePos =
              mapMouseOntoCanvas(mousePos, viewMatrix, canvasWidth, canvasHeight, getWidth(), getHeight());
        auto canvasPos = mappedMousePos.canvasPos;
        auto worldPos = mappedMousePos.worldPos;

        if ((canvasPos.x >= 0.0F && canvasPos.x < canvasWidth) && (canvasPos.y >= 0.0F && canvasPos.y < canvasHeight)) {
            unsigned int i = (canvasHeight - static_cast<unsigned int>(canvasPos.y)) * canvasWidth +
                             static_cast<unsigned int>(canvasPos.x);
            colors[i] = {1.0, 1.0, 1.0, 1.0};
            mousePositions.emplace_back(worldPos.x, worldPos.y);
        }
    }
    texture->update(colors, canvasWidth, canvasHeight);

    GL_Call(glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}
