#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <soundio/soundio.h>

#include "audio/WavLoader.h"
#include "opengl/Shader.h"

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

    void initSoundIo(int sampleRate);
};
