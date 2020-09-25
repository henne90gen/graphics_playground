#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <soundio/soundio.h>

#include "audio/WavLoader.h"
#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"

struct PlayBack {
    WavFile *wav = nullptr;
    int sampleCursor = 0;
    bool paused = false;
};

class AudioVis : public Scene {
  public:
    explicit AudioVis(SceneData &data) : Scene(data, "AudioVis"){};
    ~AudioVis() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    WavFile wav;
    PlayBack playBack = {};
    SoundIo *soundio = nullptr;
    SoundIoDevice *device = nullptr;
    SoundIoOutStream *outstream = nullptr;

    std::shared_ptr<VertexArray> va = nullptr;
    std::shared_ptr<VertexBuffer> heightBuffer = nullptr;
    std::vector<float> heightMap = {};

    void initSoundIo(int sampleRate);
    void initMesh();
    void updateMeshAmplitude(unsigned int linesPerSecond);
    void renderMesh(const glm::vec3 &modelScale, const glm::vec3 &cameraPosition, const glm::vec3 &cameraRotation,
                    bool drawWireframe);
};
