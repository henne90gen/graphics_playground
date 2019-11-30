#include "BloomEffect.h"

void BloomEffect::setup() {
    shader = std::make_shared<Shader>("scenes/bloom_effect/BloomEffectVert.glsl",
                                      "scenes/bloom_effect/BloomEffectFrag.glsl");
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();
}

void BloomEffect::destroy() {}

void BloomEffect::tick() {}
