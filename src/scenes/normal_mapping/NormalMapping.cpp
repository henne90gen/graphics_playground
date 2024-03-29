#include "NormalMapping.h"

#include "Image.h"
#include "Main.h"
#include "util/ImGuiUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

DEFINE_SCENE_MAIN(NormalMapping)
DEFINE_DEFAULT_SHADERS(normal_mapping_NormalMapping)

void NormalMapping::setup() {
    getCamera().setDistance(5);

    shader = CREATE_DEFAULT_SHADER(normal_mapping_NormalMapping);
    shader->bind();
    onAspectRatioChange();

    auto error = Model::loadFromFile("normal_mapping_resources/models/monkey.obj", shader, model);
    if (error != 0) {
        std::cout << "Failed to load model" << std::endl;
        return;
    }

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();

    std::vector<glm::vec3> tangents = {};
    std::vector<glm::vec3> biTangents = {};
    const RawMesh rawMesh = model.getRawModel().meshes[0];
    calculateTangentsAndBiTangents(rawMesh.indices, rawMesh.vertices, rawMesh.uvs, tangents, biTangents);

    std::vector<float> vertexData;
    interleaveVertexData(tangents, biTangents, vertexData);
    BufferLayout layout = {
          {ShaderDataType::Float3, "a_Tangent"},
          {ShaderDataType::Float3, "a_BiTangent"},
    };
    auto vertexBuffer = std::make_shared<VertexBuffer>(vertexData, layout);
    const OpenGLMesh mesh = model.getMeshes()[0];
    mesh.vertexArray->addVertexBuffer(vertexBuffer);

    TextureSettings textureSettings = {};
    textureSettings.dataType = GL_RGBA;
    normalMap = std::make_shared<Texture>(textureSettings);
    glActiveTexture(GL_TEXTURE1);
    normalMap->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform("u_NormalSampler", 1);
    Image normalMapImage = {};
    if (ImageOps::load("normal_mapping_resources/models/normals.jpg", normalMapImage)) {
        normalMap->update(normalMapImage.pixels.data(), normalMapImage.width, normalMapImage.height);
    }
}

void NormalMapping::destroy() {}

void NormalMapping::tick() {
    static float scale = 1.0F;
    static glm::vec3 rotation = {};
    static glm::vec3 lightPosition = {0.0, 0.0, 1.7};
    static glm::vec3 lightColor = {1.0, 1.0, 1.0};
    static float rotationSpeed = 0.01F;
    static bool rotate = true;
    static bool useNormalMap = true;

    ImGui::Begin("Settings");
    ImGui::DragFloat("Model Scale", &scale, 0.01F);
    ImGui::DragFloat3("Model Rotation", reinterpret_cast<float *>(&rotation), 0.01F);
    ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&lightPosition), 0.01F);
    ImGui::ColorEdit3("Light Color", reinterpret_cast<float *>(&lightColor));
    ImGui::DragFloat("Rotation Speed", &rotationSpeed, 0.001F);
    ImGui::Checkbox("Rotate", &rotate);
    ImGui::Checkbox("Use NormalMap", &useNormalMap);
    ImGui::End();

    if (rotate) {
        rotation.y += rotationSpeed;
    }

    const auto &viewMatrix = getCamera().getViewMatrix();

    shader->bind();
    shader->setUniform("u_Light.position", lightPosition);
    shader->setUniform("u_Light.color", lightColor);
    shader->setUniform("u_UseNormalMap", useNormalMap);
    shader->setUniform("u_View", viewMatrix);
    shader->setUniform("u_Projection", projectionMatrix);

    {
        RECORD_SCOPE_NAME("Render");
        for (const auto &mesh : model.getMeshes()) {
            renderMesh(mesh, rotation, scale);
        }
    }
}

void NormalMapping::renderMesh(const OpenGLMesh &mesh, const glm::vec3 &rotation, const float scale) {
    if (!mesh.visible) {
        return;
    }

    mesh.vertexArray->bind();

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0, 0, 1));
    shader->setUniform("u_Model", modelMatrix);

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
    shader->setUniform("u_NormalMatrix", normalMatrix);

    glActiveTexture(GL_TEXTURE0);
    shader->setUniform("u_TextureSampler", 0);
    mesh.texture->bind();

    glActiveTexture(GL_TEXTURE1);
    shader->setUniform("u_NormalSampler", 1);
    normalMap->bind();

    GL_Call(glDrawElements(GL_TRIANGLES, mesh.indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));
}

void NormalMapping::interleaveVertexData(const std::vector<glm::vec3> &tangents,
                                         const std::vector<glm::vec3> &biTangents, std::vector<float> &output) {

    for (unsigned long i = 0; i < tangents.size(); i++) {
        output.push_back(tangents[i].x);
        output.push_back(tangents[i].y);
        output.push_back(tangents[i].z);

        output.push_back(biTangents[i].x);
        output.push_back(biTangents[i].y);
        output.push_back(biTangents[i].z);
    }
}

void NormalMapping::onAspectRatioChange() {
    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
}

void NormalMapping::calculateTangentsAndBiTangents(const std::vector<glm::ivec3> &indices,
                                                   const std::vector<glm::vec3> &vertices,
                                                   const std::vector<glm::vec2> &uvs, std::vector<glm::vec3> &tangents,
                                                   std::vector<glm::vec3> &biTangents) {
    for (const auto &index : indices) {
        auto p1 = vertices[index.x];
        auto p2 = vertices[index.y];
        auto p3 = vertices[index.z];
        auto uv1 = uvs[index.x];
        auto uv2 = uvs[index.y];
        auto uv3 = uvs[index.z];

        auto edge1 = p2 - p1;
        auto edge2 = p3 - p1;
        auto deltaUV1 = uv2 - uv1;
        auto deltaUV2 = uv3 - uv1;

        float f = 1.0F / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        glm::vec3 tangent = {0.0, 0.0, 0.0};
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent = glm::normalize(tangent);
        tangents.push_back(tangent);
        tangents.push_back(tangent);
        tangents.push_back(tangent);

        glm::vec3 biTangent = {0.0, 0.0, 0.0};
        biTangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        biTangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        biTangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        biTangent = glm::normalize(biTangent);
        biTangents.push_back(biTangent);
        biTangents.push_back(biTangent);
        biTangents.push_back(biTangent);
    }
}
