#include "BloomEffect.h"

#include "Main.h"
#include "util/OpenGLUtils.h"
#include "util/RenderUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

DEFINE_SCENE_MAIN(BloomEffect)
DEFINE_DEFAULT_SHADERS(bloom_effect_BloomEffect)
DEFINE_DEFAULT_SHADERS(bloom_effect_Bloom)
DEFINE_DEFAULT_SHADERS(bloom_effect_Blur)
DEFINE_DEFAULT_SHADERS(bloom_effect_SimpleTexture)

void BloomEffect::setup() {
    shader = CREATE_DEFAULT_SHADER(bloom_effect_BloomEffect);
    shader->bind();

    shaderBlur = CREATE_DEFAULT_SHADER(bloom_effect_Blur);
    shaderBlur->bind();

    shaderBloom = CREATE_DEFAULT_SHADER(bloom_effect_Bloom);
    shaderBloom->bind();

    textureShader = CREATE_DEFAULT_SHADER(bloom_effect_SimpleTexture);
    textureShader->bind();

    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    Model::loadFromFile("bloom_effect_resources/models/monkey.obj", shader, model);

    quadVA = createQuadVA(shader);
    initLightCubeData();

    setupFramebuffers();
}

void BloomEffect::onAspectRatioChange() {
    // TODO(henne): reinitialize the color and depth buffers
    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
}

void BloomEffect::destroy() {}

void BloomEffect::tick() {
    static auto modelPosition = glm::vec3();
    static auto modelRotation = glm::vec3();
    static auto lightPosition = glm::vec3(0.5F, -0.6F, 1.5F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static float rotationSpeed = 0.006F;                      // FIXME this is highly dependent on the current framerate
    static bool rotate = true;
    static bool drawSteps = false;
    static bool doBloom = true;
    static bool doGammaCorrection = true;
    static float exposure = 1.0F;
    static float threshold = 1.0F;
    const float dragSpeed = 0.001F;

    if (rotate) {
        modelRotation.y += rotationSpeed;
    }

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Model Position", reinterpret_cast<float *>(&modelPosition), dragSpeed);
    ImGui::DragFloat3("Model Rotation", reinterpret_cast<float *>(&modelRotation), dragSpeed);
    ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&lightPosition), dragSpeed);
    ImGui::DragFloat("Rotation Speed", &rotationSpeed, dragSpeed);
    ImGui::Checkbox("Rotate", &rotate);
    ImGui::Checkbox("Draw Steps", &drawSteps);
    ImGui::Checkbox("Use Bloom", &doBloom);
    ImGui::Checkbox("Do Gamma Correction", &doGammaCorrection);
    ImGui::DragFloat("Exposure", &exposure, dragSpeed);
    ImGui::DragFloat("Threshold", &threshold, dragSpeed);
    ImGui::End();

    renderSceneToFramebuffer(modelPosition, modelRotation, lightPosition, threshold);

    unsigned int finalBlurTextureId = blurRenderedScene();

    applyBloomAndRenderAgain(finalBlurTextureId, doBloom, doGammaCorrection, exposure);

    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GL_Call(glClearColor(0.0F, 0.0F, 0.0F, 1.0F));
    GL_Call(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    renderStepsOrFinal(drawSteps);
}

void BloomEffect::setupFramebuffers() {
    RECORD_SCOPE();
    setupHdrFramebuffer();
    setupBlurringFramebuffers();
    setupFinalFramebuffer();
}

void BloomEffect::setupHdrFramebuffer() {
    RECORD_SCOPE();
    // set up two color buffers to render to
    GL_Call(glGenFramebuffers(1, &hdrFBO));
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO));
    GL_Call(glGenTextures(2, colorBuffers));
    for (unsigned int i = 0; i < 2; i++) {
        GL_Call(glBindTexture(GL_TEXTURE_2D, colorBuffers[i]));
        GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, getWidth(), getHeight(), 0, GL_RGB, GL_FLOAT, nullptr));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        // attach texture to framebuffer
        GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0));
    }

    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth = 0;
    GL_Call(glGenRenderbuffers(1, &rboDepth));
    GL_Call(glBindRenderbuffer(GL_RENDERBUFFER, rboDepth));
    GL_Call(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, getWidth(), getHeight()));
    GL_Call(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth));

    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    GL_Call(glDrawBuffers(2, attachments));

    checkFramebufferStatus();
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void BloomEffect::setupBlurringFramebuffers() {
    RECORD_SCOPE();
    // ping-pong-framebuffer for blurring
    GL_Call(glGenFramebuffers(2, pingpongFBO));
    GL_Call(glGenTextures(2, pingpongColorbuffers));

    for (unsigned int i = 0; i < 2; i++) {
        GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]));
        GL_Call(glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]));
        GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, getWidth(), getHeight(), 0, GL_RGB, GL_FLOAT, nullptr));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GL_Call(
              glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0));
        // also check if framebuffer is complete (no need for depth buffer)
        checkFramebufferStatus();
    }

    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void BloomEffect::setupFinalFramebuffer() {
    RECORD_SCOPE();
    GL_Call(glGenFramebuffers(1, &finalFbo));
    GL_Call(glGenTextures(1, &finalTexture));
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, finalFbo));
    GL_Call(glBindTexture(GL_TEXTURE_2D, finalTexture));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, getWidth(), getHeight(), 0, GL_RGB, GL_FLOAT, nullptr));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finalTexture, 0));

    checkFramebufferStatus();
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void BloomEffect::renderSceneToFramebuffer(const glm::vec3 &modelPosition, const glm::vec3 &modelRotation,
                                           const glm::vec3 &lightPosition, float threshold) {
    RECORD_SCOPE();
    static auto cameraTranslation = glm::vec3(0.5F, 0.0F, -5.0F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraRotation = glm::vec3();
    static auto ambientColor = glm::vec3(0.005F, 0.005F, 0.005F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto specularColor = glm::vec3(0.3F, 0.3F, 0.3F);      // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto lightColor = glm::vec3(1.0F);

    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO));
    GL_Call(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    shader->bind();
    shader->setUniform("u_AmbientColor", ambientColor);
    shader->setUniform("u_SpecularColor", specularColor);
    shader->setUniform("u_Light.position", lightPosition);
    shader->setUniform("u_Light.color", lightColor);
    shader->setUniform("u_Threshold", threshold);

    glm::mat4 viewMatrix = createViewMatrix(cameraTranslation, cameraRotation);
    drawModel(viewMatrix, modelPosition, modelRotation);

    drawLightCube(viewMatrix, lightPosition);
}

unsigned int BloomEffect::blurRenderedScene() {
    RECORD_SCOPE();
    bool horizontal = true;
    bool first_iteration = true;
    unsigned int amount = 10;
    shaderBlur->bind();
    for (unsigned int i = 0; i < amount; i++) {
        GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]));
        shaderBlur->setUniform("horizontal", horizontal);
        // bind texture of other framebuffer (or scene if first iteration)
        if (first_iteration) {
            GL_Call(glBindTexture(GL_TEXTURE_2D, colorBuffers[1]));
        } else {
            GL_Call(glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]));
        }

        renderQuad(shaderBlur);

        horizontal = !horizontal;
        if (first_iteration) {
            first_iteration = false;
        }
    }
    return pingpongColorbuffers[!horizontal];
}

void BloomEffect::applyBloomAndRenderAgain(const unsigned int blurTexture, bool doBloom, bool doGammaCorrection,
                                           float exposure) {
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, finalFbo));
    GL_Call(glClearColor(0.2F, 0.2F, 0.2F, 1.0F));
    GL_Call(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    shaderBloom->bind();
    GL_Call(glActiveTexture(GL_TEXTURE0));
    GL_Call(glBindTexture(GL_TEXTURE_2D, colorBuffers[0]));
    GL_Call(glActiveTexture(GL_TEXTURE1));
    GL_Call(glBindTexture(GL_TEXTURE_2D, blurTexture));
    shaderBloom->setUniform("u_SceneTexture", 0);
    shaderBloom->setUniform("u_BloomBlurTexture", 1);
    shaderBloom->setUniform("u_DoBloom", doBloom);
    shaderBloom->setUniform("u_DoGammaCorrection", doGammaCorrection);
    shaderBloom->setUniform("u_Exposure", exposure);
    shaderBloom->setUniform("u_ModelMatrix", glm::scale(glm::mat4(1.0F), glm::vec3(2.0F)));
    renderQuad(shaderBloom);
}

void BloomEffect::drawModel(const glm::mat4 &viewMatrix, const glm::vec3 &modelTranslation,
                            const glm::vec3 &modelRotation) const {
    shader->bind();
    shader->setUniform("u_View", viewMatrix);
    shader->setUniform("u_Projection", projectionMatrix);
    shader->setUniform("u_TextureSampler", 0);
    for (auto &mesh : model.getMeshes()) {
        if (!mesh.visible) {
            continue;
        }

        mesh.vertexArray->bind();

        glm::mat4 modelMatrix = glm::mat4(1.0F);
        modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
        modelMatrix = glm::translate(modelMatrix, modelTranslation);
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
        shader->setUniform("u_Model", modelMatrix);
        shader->setUniform("u_NormalMatrix", normalMatrix);

        mesh.texture->bind();

        GL_Call(glDrawElements(GL_TRIANGLES, mesh.indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

        mesh.vertexArray->unbind();
    }
    shader->unbind();
}

void BloomEffect::drawLightCube(const glm::mat4 &viewMatrix, const glm::vec3 &lightPosition) const {
    const float cubeScale = 0.5F;
    textureShader->bind();
    GL_Call(glActiveTexture(GL_TEXTURE0));
    staticRedTexture->bind();
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0F), lightPosition);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(cubeScale));
    textureShader->setUniform("u_ProjectionMatrix", projectionMatrix);
    textureShader->setUniform("u_ViewMatrix", viewMatrix);
    textureShader->setUniform("u_ModelMatrix", modelMatrix);
    textureShader->setUniform("u_Texture", 0);
    cubeVA->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, cubeVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    cubeVA->unbind();
    textureShader->setUniform("u_ProjectionMatrix", glm::mat4(1.0F));
    textureShader->setUniform("u_ViewMatrix", glm::mat4(1.0F));
    textureShader->unbind();
}

void BloomEffect::renderQuad(const std::shared_ptr<Shader> &s) {
    quadVA->setShader(s);

    quadVA->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    quadVA->unbind();
}

void BloomEffect::renderStepsOrFinal(bool drawSteps) {
    textureShader->bind();
    textureShader->setUniform("u_Texture", 0);
    textureShader->setUniform("u_ColorAmplifier", 1.0F);
    GL_Call(glActiveTexture(GL_TEXTURE0));

    if (drawSteps) {
        glm::mat4 topLeft = glm::translate(glm::mat4(1.0), glm::vec3(-0.5, 0.5, 0.0));
        GL_Call(glBindTexture(GL_TEXTURE_2D, colorBuffers[0]));
        textureShader->setUniform("u_ModelMatrix", topLeft);
        renderQuad(textureShader);

        glm::mat4 topRight = glm::translate(glm::mat4(1.0), glm::vec3(0.5, 0.5, 0.0));
        GL_Call(glBindTexture(GL_TEXTURE_2D, colorBuffers[1]));
        textureShader->setUniform("u_ModelMatrix", topRight);
        renderQuad(textureShader);

        textureShader->setUniform("u_ColorAmplifier", 10.0F);
        glm::mat4 bottomLeft = glm::translate(glm::mat4(1.0), glm::vec3(-0.5, -0.5, 0.0));
        GL_Call(glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[0]));
        textureShader->setUniform("u_ModelMatrix", bottomLeft);
        renderQuad(textureShader);

        glm::mat4 bottomRight = glm::translate(glm::mat4(1.0), glm::vec3(0.5, -0.5, 0.0));
        GL_Call(glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[1]));
        textureShader->setUniform("u_ModelMatrix", bottomRight);
        renderQuad(textureShader);
        textureShader->setUniform("u_ColorAmplifier", 1.0F);
    } else {
        glm::mat4 modelMatrix = glm::mat4(1.0);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0F));
        GL_Call(glBindTexture(GL_TEXTURE_2D, finalTexture));
        textureShader->setUniform("u_ModelMatrix", modelMatrix);
        renderQuad(textureShader);
    }
}

void BloomEffect::initLightCubeData() {
    cubeVA = createCubeVA(textureShader);
    cubeVA->bind();
    std::vector<glm::vec2> uvs = {};
    for (unsigned int i = 0; i < 6; i++) {
        uvs.emplace_back(0, 0);
        uvs.emplace_back(1, 0);
        uvs.emplace_back(1, 1);
        uvs.emplace_back(0, 1);
    }
    BufferLayout layout = {
          {Float2, "a_UV"},
    };
    std::shared_ptr<VertexBuffer> vb = std::make_shared<VertexBuffer>(uvs, layout);
    cubeVA->addVertexBuffer(vb);
    staticRedTexture = std::make_shared<Texture>();
    std::vector<glm::vec3> pixels = {{1.0, 0.0, 0.0}};
    staticRedTexture->update(pixels, 1, 1);
}
