#include "Tessellation.h"

#include "Main.h"
#include "util/RenderUtils.h"

DEFINE_SCENE_MAIN(Tessellation)
DEFINE_DEFAULT_SHADERS(tessellation_Tessellation)
DEFINE_TESS_CONTROL_SHADER(tessellation_Tessellation)
DEFINE_TESS_EVALUATION_SHADER(tessellation_Tessellation)

constexpr float FIELD_OF_VIEW = 45.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 10000.0F;

void Tessellation::setup() {
    getCamera().setDistance(400);
    getCamera().setRotation(0.4F, 2.3F);
    getCamera().setFocalPoint(glm::vec3(40, 200, 80));

    shader = std::make_shared<Shader>();
    shader->attachVertexShader(SHADER_CODE(tessellation_TessellationVert));
    shader->attachFragmentShader(SHADER_CODE(tessellation_TessellationFrag));
    shader->attachTessControlShader(SHADER_CODE(tessellation_TessellationTcs));
    shader->attachTessEvaluationShader(SHADER_CODE(tessellation_TessellationTes));
    shader->compile();

    simpleShader = CREATE_DEFAULT_SHADER(tessellation_Tessellation);

    glm::vec2 uvMin = {0.0F, 0.0F};
    glm::vec2 uvMax = {1.0F, 1.0F};
    std::vector<float> quadVertices = {
          0.0F, 0.0F, 0.0F, uvMin.x, uvMin.y, 0.0F, 1.0F, 0.0F, //
          1.0F, 0.0F, 0.0F, uvMax.x, uvMin.y, 0.0F, 1.0F, 0.0F, //
          1.0F, 0.0F, 1.0F, uvMax.x, uvMax.y, 0.0F, 1.0F, 0.0F, //
          0.0F, 0.0F, 1.0F, uvMin.x, uvMax.y, 0.0F, 1.0F, 0.0F, //
    };

    int width = 10;
    int height = 10;
    std::vector<float> vertices = {};
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            for (size_t i = 0; i < quadVertices.size(); i++) {
                float f = quadVertices[i];
                if (i % 8 == 0) {
                    f += static_cast<float>(row);
                    f *= 100.0F;
                } else if (i % 8 == 2) {
                    f += static_cast<float>(col);
                    f *= 100.0F;
                }
                vertices.push_back(f);
            }
        }
    }

    va = std::make_shared<VertexArray>(shader);
    BufferLayout bufferLayout = {
          {ShaderDataType::Float3, "position_in"},
          {ShaderDataType::Float2, "uv_in"},
          {ShaderDataType::Float3, "normal_in"},
    };
    auto buffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    va->addVertexBuffer(buffer);

    std::vector<glm::ivec3> indices = {};
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            const int i = (row * width + col) * 4;
            indices.emplace_back(i, i + 1, i + 2);
            indices.emplace_back(i, i + 2, i + 3);
        }
    }
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    va->setIndexBuffer(indexBuffer);
}

void Tessellation::destroy() {}

void Tessellation::tick() {
    static auto model = ModelSpace();
    static auto drawWireframe = true;
    static TessellationLevels tessellationLevels = {};

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Model Position", reinterpret_cast<float *>(&model.position), 0.01F);
    ImGui::DragFloat3("Model Rotation", reinterpret_cast<float *>(&model.rotation), 0.01F);
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&model.scale), 0.01F);
    ImGui::Checkbox("Draw Wireframe", &drawWireframe);
    ImGui::DragFloat3("Outer Tess", reinterpret_cast<float *>(&tessellationLevels.outer), 1.0F, 1.0F, 100.0F);
    ImGui::DragFloat("Inner Tess", &tessellationLevels.inner, 1.0F, 1.0F, 100.0F);
    ImGui::End();

    renderTessellatedQuad(drawWireframe, model, tessellationLevels);
}

void Tessellation::renderTessellatedQuad(bool drawWireframe, const ModelSpace &model,
                                         const TessellationLevels &levels) {
    va->bind();

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::translate(modelMatrix, model.position);
    modelMatrix = glm::rotate(modelMatrix, model.rotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, model.rotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, model.rotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, model.scale);
    auto viewMatrix = getCamera().getViewMatrix();
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    shader->bind();
    shader->setUniform("outerTess", levels.outer);
    shader->setUniform("innerTess", levels.inner);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    if (drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    GL_Call(glPatchParameteri(GL_PATCH_VERTICES, 3));
    GL_Call(glDrawElements(GL_PATCHES, va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    if (drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }
}
