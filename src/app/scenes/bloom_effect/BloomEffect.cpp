#include "BloomEffect.h"

#include "util/RenderUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

void BloomEffect::setup() {
    shader = std::make_shared<Shader>("scenes/bloom_effect/BloomEffectVert.glsl",
                                      "scenes/bloom_effect/BloomEffectFrag.glsl");
    shader->bind();

    shaderBlur = std::make_shared<Shader>("scenes/bloom_effect/BlurVert.glsl", "scenes/bloom_effect/BlurFrag.glsl");
    shaderBlur->bind();

    shaderBloom = std::make_shared<Shader>("scenes/bloom_effect/BloomVert.glsl", "scenes/bloom_effect/BloomFrag.glsl");
    shaderBloom->bind();

    textureShader = std::make_shared<Shader>("scenes/bloom_effect/SimpleTextureVert.glsl",
                                             "scenes/bloom_effect/SimpleTextureFrag.glsl");
    textureShader->bind();

    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    model = std::make_unique<Model>();
    model->loadFromFile("scenes/bloom_effect/models/monkey.obj", shader);

    quadVA = createQuadVA(shader);

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
    unsigned int rboDepth;
    GL_Call(glGenRenderbuffers(1, &rboDepth));
    GL_Call(glBindRenderbuffer(GL_RENDERBUFFER, rboDepth));
    GL_Call(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, getWidth(), getHeight()));
    GL_Call(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth));

    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    GL_Call(glDrawBuffers(2, attachments));

    // finally check if framebuffer is complete
    GLenum status;
    GL_Call(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete! (" << status << ")" << std::endl;
    }
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));

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
        GL_Call(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Framebuffer not complete!" << std::endl;
        }
    }
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));

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
    // also check if framebuffer is complete (no need for depth buffer)
    GL_Call(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete!" << std::endl;
    }
    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void BloomEffect::onAspectRatioChange() {
    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
}

void BloomEffect::destroy() {}

void BloomEffect::tick() {
    static auto cameraTranslation = glm::vec3(0.5F, 0.0F, -5.0F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraRotation = glm::vec3();
    static auto modelTranslation = glm::vec3();
    static auto modelRotation = glm::vec3();
    static float scale = 1.0F;
    static bool drawWireframe = false;
    static auto ambientColor = glm::vec3(0.05F, 0.05F, 0.05F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto specularColor = glm::vec3(0.3F, 0.3F, 0.3F);   // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto lightPosition = glm::vec3(0.0F, -0.7F, 1.6F);  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto lightColor = glm::vec3(1.0F);
    static bool bloom = true;
    static float exposure = 1.0;
    static glm::vec3 pos = {0.0F, 0.0F, 0.0F};

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&pos), 0.001F);
    ImGui::Checkbox("Use Bloom", &bloom);
    ImGui::End();

    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO));
    GL_Call(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    shader->bind();
    shader->setUniform("u_AmbientColor", ambientColor);
    shader->setUniform("u_SpecularColor", specularColor);
    shader->setUniform("u_Light.position", lightPosition);
    shader->setUniform("u_Light.color", lightColor);
    drawModel(scale, modelTranslation, modelRotation, cameraRotation, cameraTranslation, drawWireframe);

    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));

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

        shaderBlur->setUniform("u_ModelMatrix", glm::mat4(1.0));
        renderQuad(shaderBlur);

        horizontal = !horizontal;
        if (first_iteration) {
            first_iteration = false;
        }
    }

    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, finalFbo));
    GL_Call(glClearColor(0.2F, 0.2F, 0.2F, 1.0F));
    GL_Call(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    shaderBloom->bind();
    GL_Call(glActiveTexture(GL_TEXTURE0));
    GL_Call(glBindTexture(GL_TEXTURE_2D, colorBuffers[0]));
    GL_Call(glActiveTexture(GL_TEXTURE1));
    GL_Call(glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]));
    shaderBloom->setUniform("bloom", bloom);
    shaderBloom->setUniform("exposure", exposure);
    renderQuad(shaderBloom);

    GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GL_Call(glClearColor(0.0F, 0.0F, 0.0F, 1.0F));
    GL_Call(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    renderAllQuads(pos);
}

void BloomEffect::drawModel(float scale, const glm::vec3 &modelTranslation, const glm::vec3 &modelRotation,
                            const glm::vec3 &cameraRotation, const glm::vec3 &cameraTranslation,
                            const bool drawWireframe) const {
    shader->bind();
    for (auto &mesh : model->getMeshes()) {
        if (!mesh->visible) {
            continue;
        }

        mesh->vertexArray->bind();

        glm::mat4 modelMatrix = glm::mat4(1.0F);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
        modelMatrix = glm::translate(modelMatrix, modelTranslation);
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
        glm::mat4 viewMatrix = createViewMatrix(cameraTranslation, cameraRotation);
        shader->setUniform("u_Model", modelMatrix);
        shader->setUniform("u_NormalMatrix", normalMatrix);
        shader->setUniform("u_View", viewMatrix);
        shader->setUniform("u_Projection", projectionMatrix);

        shader->setUniform("u_TextureSampler", 0);
        mesh->texture->bind();

        if (drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        }

        GL_Call(glDrawElements(GL_TRIANGLES, mesh->indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

        if (drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        mesh->vertexArray->unbind();
    }
    shader->unbind();
}

void BloomEffect::renderQuad(const std::shared_ptr<Shader> &s) {
    quadVA->setShader(s);

    quadVA->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    quadVA->unbind();
}

void BloomEffect::renderAllQuads(const glm::vec3 &pos) {
    glm::mat4 modelMatrix = glm::mat4(1.0);
    //    float scale = 0.25F;
    //    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale, scale, scale));

    textureShader->setUniform("u_Texture", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
    textureShader->setUniform("u_ModelMatrix", modelMatrix);
    //    renderQuad(textureShader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffers[1]);
    textureShader->setUniform("u_ModelMatrix", modelMatrix);
    //    renderQuad(textureShader);

    glBindTexture(GL_TEXTURE_2D, finalTexture);
    glm::mat4 topLeft = glm::translate(modelMatrix, pos);
    textureShader->setUniform("u_ModelMatrix", topLeft);
    renderQuad(textureShader);
}
