#include "SpotLight.h"

#include "Main.h"
#include "util/RenderUtils.h"

#include <array>
#include <glm/gtx/rotate_vector.hpp>

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

DEFINE_SCENE_MAIN(SpotLight)
DEFINE_DEFAULT_SHADERS(spot_light_SpotLight)

void SpotLight::setup() {
    shader = CREATE_DEFAULT_SHADER(spot_light_SpotLight);
    shader->bind();
    onAspectRatioChange();
    quadVA = createWalls();
}

void SpotLight::onAspectRatioChange() {
    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
}

void SpotLight::destroy() {}

void SpotLight::tick() {
    static bool wireframe = false;
    static glm::vec3 cameraPosition = {0.0F, 0.0F, -1.0F};
    static glm::vec3 cameraRotation = {0.0F, 1.5F, 0.0F};
    static glm::vec3 modelPosition = {-5.0F, 0.0F, 0.0F};
    static glm::vec3 ambientColor = {0.1F, 0.1f, 0.1F};
    static glm::vec3 lightColor = {1.0F, 1.0F, 1.0F};
    static bool useAmbient = true;
    static bool useDiffuse = true;
    static bool useSpecular = true;
    static int falloffType = 0;
    static float falloffPosition = 0.4F;
    static float falloffSpeed = 10.0F;
    static float walkSpeed = 2.0F;
    static float rotationSpeed = 2.0F;
    const float dragSpeed = 0.01F;

    ImGui::Begin("Settings");
    ImGui::Checkbox("Wireframe", &wireframe);
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition), dragSpeed);
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), dragSpeed);
    ImGui::DragFloat3("Model Position", reinterpret_cast<float *>(&modelPosition), dragSpeed);
    ImGui::ColorEdit3("Light Color", reinterpret_cast<float *>(&lightColor), dragSpeed);
    ImGui::ColorEdit3("Ambient Color", reinterpret_cast<float *>(&ambientColor), dragSpeed);
    ImGui::Checkbox("Use Ambient", &useAmbient);
    ImGui::Checkbox("Use Diffuse", &useDiffuse);
    ImGui::Checkbox("Use Specular", &useSpecular);
    static const std::array<const char *, 3> items = {"Cosine", "Sigmoid", "Linear"};
    ImGui::Combo("Falloff Type", &falloffType, items.data(), items.size());
    ImGui::SliderFloat("Falloff Position", &falloffPosition, 0.0F, 1.0F);
    ImGui::SliderFloat("Falloff Speed", &falloffSpeed, 1.0F, 50.0F);
    ImGui::DragFloat("Walk Speed", &walkSpeed, dragSpeed);
    ImGui::DragFloat("Rotation Speed", &rotationSpeed, dragSpeed);
    ImGui::End();

    moveKeyboardOnly(cameraPosition, cameraRotation, walkSpeed, rotationSpeed);

    shader->bind();
    quadVA->bind();
    if (wireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    auto modelMatrix = createModelMatrix(modelPosition);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
    auto viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    shader->setUniform("u_ModelMatrix", modelMatrix);
    shader->setUniform("u_NormalMatrix", normalMatrix);
    shader->setUniform("u_ViewMatrix", viewMatrix);
    shader->setUniform("u_ProjectionMatrix", projectionMatrix);
    shader->setUniform("u_AmbientColor", ambientColor);
    //    shader->setUniform("u_SpecularColor", specularColor);
    shader->setUniform("u_UseAmbient", useAmbient);
    shader->setUniform("u_UseDiffuse", useDiffuse);
    shader->setUniform("u_UseSpecular", useSpecular);

    // setup light
    shader->setUniform("u_LightColor", lightColor);
    shader->setUniform("u_LightPosition", cameraPosition * -1.0F);
    auto cameraDirection = calculateDirectionFromRotation(cameraRotation);
    cameraDirection.z *= -1.0F;
    shader->setUniform("u_LightDirection", cameraDirection);
    shader->setUniform("u_FalloffType", falloffType);
    shader->setUniform("u_FalloffPosition", falloffPosition);
    shader->setUniform("u_FalloffSpeed", falloffSpeed);

    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    if (wireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    quadVA->unbind();
    shader->unbind();
}

void SpotLight::moveKeyboardOnly(glm::vec3 &position, glm::vec3 &rotation, float walkSpeed, float rotationSpeed) {
    double timeDelta = getLastFrameTime();
    InputData *data = getInput();
    float forward = 0.0F;

    if (data->keyboard.isKeyDown('W')) {
        forward = 1.0F;
    }
    if (data->keyboard.isKeyDown('S')) {
        forward = -1.0F;
    }

    if (forward != 0.0F) {
        auto rot = glm::vec3(rotation.x * -1.0F, rotation.y * -1.0F, rotation.z);
        auto cameraDirection = calculateDirectionFromRotation(rot);
        const glm::vec3 movement = cameraDirection * static_cast<float>(forward * timeDelta * walkSpeed);
        position += movement;
    }

    float around = 0.0F;
    if (data->keyboard.isKeyDown('A')) {
        around = -1.0F;
    }
    if (data->keyboard.isKeyDown('D')) {
        around = 1.0F;
    }
    rotation.y += around * timeDelta * rotationSpeed;
}

void addWall(std::vector<float> &vertices, const glm::vec3 &position, const glm::vec3 &normal) {
#define ADD_VERTEX(v)                                                                                                  \
    vertices.push_back((v).x);                                                                                           \
    vertices.push_back((v).y);                                                                                           \
    vertices.push_back((v).z);                                                                                           \
    vertices.push_back(normal.x);                                                                                      \
    vertices.push_back(normal.y);                                                                                      \
    vertices.push_back(normal.z)
    auto v1 = glm::vec4(-0.5F, 0.0F, -0.5F, 1.0F);
    auto v2 = glm::vec4(0.5F, 0.0F, -0.5F, 1.0F);
    auto v3 = glm::vec4(0.5F, 0.0F, 0.5F, 1.0F);
    auto v4 = glm::vec4(-0.5F, 0.0F, 0.5F, 1.0F);

    // NOTE: the winding order of the vertices is not correct, for the case that the normal is pointing straight down
    if (normal != glm::vec3(0.0F, -1.0F, 0.0F)) {
        auto rotationMatrix = glm::orientation(normal, glm::vec3(0.0F, 1.0F, 0.0F));
        v1 = rotationMatrix * v1;
        v2 = rotationMatrix * v2;
        v3 = rotationMatrix * v3;
        v4 = rotationMatrix * v4;
    }

    auto positionV4 = glm::vec4(position.x, position.y, position.z, 1.0F);
    v1 += positionV4;
    v2 += positionV4;
    v3 += positionV4;
    v4 += positionV4;

    ADD_VERTEX(v1);
    ADD_VERTEX(v2);
    ADD_VERTEX(v3);
    ADD_VERTEX(v4);
}

std::shared_ptr<VertexArray> SpotLight::createWalls() {
    const auto northNormal = glm::vec3(0.0F, 0.0F, -1.0F);
    const auto southNormal = glm::vec3(0.0F, 0.0F, 1.0F);
    const auto eastNormal = glm::vec3(1.0F, 0.0F, 0.0F);
    const auto westNormal = glm::vec3(-1.0F, 0.0F, 0.0F);
    const auto floorNormal = glm::vec3(0.0F, 1.0F, 0.0F);
    const auto ceilingNormal = glm::vec3(0.0F, -1.0F, 0.0F);
    const auto northOffset = glm::vec3(0.0F, 0.0F, 0.5F);
    const auto southOffset = glm::vec3(0.0F, 0.0F, -0.5F);
    const auto eastOffset = glm::vec3(-0.5F, 0.0F, 0.0F);
    const auto westOffset = glm::vec3(0.5F, 0.0F, 0.0F);
    const auto floorOffset = glm::vec3(0.0F, -0.5F, 0.0F);
    const auto ceilingOffset = glm::vec3(0.0F, 0.5F, 0.0F);
    const std::string map = "OOOOOOOOOOOOOOOOOOOO"
                            "OOOXXXXXXXXXXXXXXOOO"
                            "OOOOOXOOOOOOOOXOOOOO"
                            "OOOOOXOOOOOXXXXOOOOO"
                            "OOOOOXXXXXXXOOOOOOOO"
                            "OOOOOOOOOOOOOOOOOOOO";
    const unsigned int width = 20;
    const unsigned int height = map.size() / width;

    std::vector<float> vertices = {};
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            char currentChar = map[y * width + x];
            if (currentChar != 'X') {
                continue;
            }

            auto position = glm::vec3(x, 0, y);
            addWall(vertices, position + floorOffset, floorNormal);
            addWall(vertices, position + ceilingOffset, ceilingNormal);

            char left = map[y * width + (x - 1)];
            if (left == 'O') {
                addWall(vertices, position + eastOffset, eastNormal);
            }

            char right = map[y * width + (x + 1)];
            if (right == 'O') {
                addWall(vertices, position + westOffset, westNormal);
            }

            char top = map[(y - 1) * width + x];
            if (top == 'O') {
                addWall(vertices, position + southOffset, southNormal);
            }

            char bottom = map[(y + 1) * width + x];
            if (bottom == 'O') {
                addWall(vertices, position + northOffset, northNormal);
            }
        }
    }

    auto result = std::make_shared<VertexArray>(shader);
    BufferLayout bufferLayout = {
          {ShaderDataType::Float3, "a_Position"},
          {ShaderDataType::Float3, "a_Normal"},
    };
    auto buffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    result->addVertexBuffer(buffer);

    std::vector<glm::ivec3> indices = {};
    const unsigned int wallCount = vertices.size() / (6 * 4);
    for (unsigned int i = 0; i < wallCount * 4; i += 4) {
        indices.emplace_back(i, i + 1, i + 2); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        indices.emplace_back(i, i + 2, i + 3); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    }

    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    result->setIndexBuffer(indexBuffer);

    return result;
}

glm::vec3 calculateDirectionFromRotation(const glm::vec3 &rotation) {
    auto rotationMatrix = glm::mat4(1.0F);
    rotationMatrix = glm::rotate(rotationMatrix, rotation.x, glm::vec3(1, 0, 0));
    rotationMatrix = glm::rotate(rotationMatrix, rotation.y, glm::vec3(0, 1, 0));
    rotationMatrix = glm::rotate(rotationMatrix, rotation.z, glm::vec3(0, 0, 1));
    auto cameraDirectionV4 = rotationMatrix * glm::vec4(0.0F, 0.0F, 1.0F, 1.0F);
    auto cameraDirection = glm::vec3(cameraDirectionV4.x, cameraDirectionV4.y, cameraDirectionV4.z);
    return glm::normalize(cameraDirection);
}
