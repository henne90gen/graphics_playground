#pragma once

#include "Scene.h"

#include <fourier_transform/Fourier.h>
#include <functional>
#include <soundio/soundio.h>

#include "audio/WavLoader.h"
#include "fourier_transform/Fourier.h"
#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"

struct PlayBack {
    WavFile *wav = nullptr;
    std::vector<fourier::DataPoint> coefficients = {};
    int sampleCursor = 0;
    bool paused = false;
};

class AudioVis : public Scene {
  public:
    enum class VisMode { AMPLITUDE = 0, PHASE = 1, FREQUENCY = 2, MAGNITUDE = 3 };

    explicit AudioVis() : Scene("AudioVis"){};
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
    void renderMesh(const glm::vec3 &modelScale, const glm::vec3 &cameraPosition, const glm::vec3 &cameraRotation,
                    bool drawWireframe);
    void updateMesh(const std::function<double(int)> &calcSample01Func, unsigned int linesPerSecond);
    void updateMeshAmplitude(unsigned int linesPerSecond);
    void updateMeshPhase(unsigned int linesPerSecond);
    void updateMeshFrequency(unsigned int linesPerSecond);
    void updateMeshMagnitude(unsigned int linesPerSecond);
};
