#include "AudioVis.h"

#include <algorithm>
#include <array>

#include "Main.h"

constexpr float FIELD_OF_VIEW = 45.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 100000.0F;

constexpr auto WIDTH = 50;
constexpr auto LENGTH = 100;

DEFINE_SCENE_MAIN(AudioVis)
DEFINE_DEFAULT_SHADERS(audio_vis_AudioVis)

void AudioVis::setup() {
    shader = CREATE_DEFAULT_SHADER(audio_vis_AudioVis);

    if (!loadWavFile("audio_vis_resources/res/audio_test.wav", wav)) {
        return;
    }

    playBack.wav = &wav;
    auto inputData = std::vector<float>(wav.data.subChunkSize);
    for (unsigned int i = 0; i < wav.data.subChunkSize; i++) {
        inputData[i] = static_cast<float>(wav.data.data8[i]);
    }
    playBack.coefficients = fourier::fft(inputData, playBack.wav->header.sampleRate);

    initSoundIo(wav.header.sampleRate);
    initMesh();

    getCamera().setFocalPoint(glm::vec3(35, 0, 0));
    getCamera().setRotation(0.25, -0.7);
    getCamera().setDistance(50);
}

void AudioVis::destroy() {
    std::free(wav.data.data8);
    soundio_outstream_destroy(outstream);
    soundio_device_unref(device);
    soundio_destroy(soundio);
}

void AudioVis::tick() {
    if (playBack.wav == nullptr) {
        return;
    }

    static auto modelScale = glm::vec3(1.0F, 20.0F, -1.0F);
    static bool drawWireframe = true;
    static int linesPerSecond = 15;
    static VisMode currentMode = VisMode::AMPLITUDE;

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), 0.001F);
    ImGui::Checkbox("Wireframe", &drawWireframe);

    ImGui::Separator();

    ImGui::Text("Sample Rate: %d", wav.header.sampleRate);
    ImGui::Text("Bits per Sample: %d", wav.header.bitsPerSample);
    ImGui::Text("Number of Channels: %d", wav.header.numChannels);

    ImGui::Text("Cursor: %d", playBack.sampleCursor);
    const auto sampleRate = (static_cast<float>(wav.header.sampleRate) * static_cast<float>(wav.header.numChannels));
    if (sampleRate != 0) {
        const auto seconds = static_cast<float>(playBack.sampleCursor) / sampleRate;
        ImGui::Text("%.2fs", seconds);
    }

    if (wav.header.numChannels != 0) {
        const auto cursorMax = static_cast<int>(wav.data.subChunkSize) / wav.header.numChannels;
        ImGui::SliderInt("##", &playBack.sampleCursor, 0, cursorMax, "");
    }

    std::string btnText = "Pause";
    if (playBack.paused) {
        btnText = "Play";
    }
    if (ImGui::Button(btnText.c_str())) {
        playBack.paused = !playBack.paused;
        soundio_outstream_pause(outstream, playBack.paused);
    }

    ImGui::DragInt("Lines Per Second", &linesPerSecond, 1, 1, wav.header.sampleRate);

    std::array<const char *, 4> items = {"AMPLITUDE", "PHASE", "FREQUENCY", "MAGNITUDE"};
    ImGui::Combo("Current Mode", reinterpret_cast<int *>(&currentMode),
                 reinterpret_cast<const char *const *>(items.data()), items.size());

    ImGui::End();

#if !EMSCRIPTEN
    switch (currentMode) {
    case VisMode::AMPLITUDE:
        updateMeshAmplitude(linesPerSecond);
        break;
    case VisMode::PHASE:
        updateMeshPhase(linesPerSecond);
        break;
    case VisMode::FREQUENCY:
        updateMeshFrequency(linesPerSecond);
        break;
    case VisMode::MAGNITUDE:
        updateMeshMagnitude(linesPerSecond);
        break;
    }

    renderMesh(modelScale, drawWireframe);
#endif
}

void writeCallback(struct SoundIoOutStream *outstream, int frameCountMin, int frameCountMax) {
    auto *playBack = reinterpret_cast<PlayBack *>(outstream->userdata);
    const SoundIoChannelLayout *layout = &outstream->layout;
    SoundIoChannelArea *areas = nullptr;

    std::cout << "max: " << frameCountMax << ", min: " << frameCountMin << std::endl;

    int framesLeft = frameCountMax;
    while (framesLeft > 0) {
        int frameCount = framesLeft;
        int err = soundio_outstream_begin_write(outstream, &areas, &frameCount);
        if (err != 0) {
            std::cerr << soundio_strerror(err) << std::endl;
            exit(1);
        }
        if (frameCount == 0) {
            break;
        }

        bool playBackEnded = false;
        for (int frame = 0; frame < frameCount; frame++) {
            for (int channel = 0; channel < layout->channel_count; channel++) {
                unsigned int sampleOffsetBytes = playBack->sampleCursor * 2;
                if (sampleOffsetBytes >= playBack->wav->data.subChunkSize) {
                    playBackEnded = true;
                    break;
                }

                int16_t *samplePtr = playBack->wav->data.data16 + playBack->sampleCursor;
                auto *ptr = (int16_t *)(areas[channel].ptr + areas[channel].step * frame);
                *ptr = *samplePtr;

                playBack->sampleCursor++;
            }

            if (playBackEnded) {
                break;
            }
        }

        err = soundio_outstream_end_write(outstream);
        if (err != 0) {
            std::cerr << soundio_strerror(err) << std::endl;
            exit(1);
        }

        if (playBackEnded) {
            playBack->sampleCursor = playBack->wav->data.subChunkSize / playBack->wav->header.numChannels;
            playBack->paused = true;
            err = soundio_outstream_pause(outstream, true);
            if (err != 0) {
                std::cerr << soundio_strerror(err) << std::endl;
                exit(1);
            }
            break;
        }

        framesLeft -= frameCount;
    }
}

void AudioVis::initSoundIo(int sampleRate) {
    soundio = soundio_create();
    if (soundio == nullptr) {
        std::cerr << "out of memory" << std::endl;
        return;
    }

    int err = 0;
    for (int i = 0; i < soundio_backend_count(soundio); i++) {
        auto backend = soundio_get_backend(soundio, i);
        if (backend == SoundIoBackendJack) {
            // JACK caused problems when pipewire was installed
            continue;
        }

        auto backend_name = soundio_backend_name(backend);
        std::cout << "Using backend " << backend_name << std::endl;

        err = soundio_connect_backend(soundio, backend);
        if (err != 0) {
            std::cerr << "error connecting: " << soundio_strerror(err) << std::endl;
            return;
        }
        break;
    }

    soundio_flush_events(soundio);

    const auto default_out_device_index = soundio_default_output_device_index(soundio);
    if (default_out_device_index < 0) {
        std::cerr << "no output device found" << std::endl;
        return;
    }

    device = soundio_get_output_device(soundio, default_out_device_index);
    if (device == nullptr) {
        std::cerr << "out of memory" << std::endl;
        return;
    }

    std::cout << "Output device: " << device->name << std::endl;

    outstream = soundio_outstream_create(device);
    if (outstream == nullptr) {
        std::cerr << "out of memory" << std::endl;
        return;
    }

    outstream->format = SoundIoFormatS16LE;
    outstream->sample_rate = sampleRate;
    outstream->userdata = &playBack;
    outstream->write_callback = writeCallback;

    err = soundio_outstream_open(outstream);
    if (err != 0) {
        std::cerr << "unable to open stream: " << soundio_strerror(err) << std::endl;
        return;
    }

    if (outstream->layout_error != 0) {
        std::cerr << "unable to set channel layout: " << soundio_strerror(outstream->layout_error) << std::endl;
    }

    err = soundio_outstream_start(outstream);
    if (err != 0) {
        std::cerr << "unable to start stream: " << soundio_strerror(err) << std::endl;
        return;
    }

    // pause the audio playback by default
    playBack.paused = true;
    err = soundio_outstream_pause(outstream, playBack.paused);
    if (err != 0) {
        std::cerr << "unable to pause stream" << std::endl;
        return;
    }

    std::cout << "Successfully initialized libsoundio" << std::endl;
}

void AudioVis::initMesh() {
    va = std::make_shared<VertexArray>(shader);
    va->bind();

    const unsigned int verticesCount = WIDTH * LENGTH;
    std::vector<glm::vec2> vertices = std::vector<glm::vec2>(verticesCount);
    for (unsigned long i = 0; i < vertices.size(); i++) {
        auto x = static_cast<float>(i % WIDTH);
        auto y = std::floor(static_cast<float>(i) / static_cast<float>(WIDTH));
        vertices[i] = glm::vec2(x, y);
    }

    const unsigned long verticesSize = vertices.size() * 2 * sizeof(float);
    const BufferLayout positionLayout = {{ShaderDataType::Float2, "position"}};
    auto positionBuffer = std::make_shared<VertexBuffer>(vertices.data(), verticesSize, positionLayout);
    va->addVertexBuffer(positionBuffer);

    const unsigned int heightMapCount = WIDTH * LENGTH;
    heightMap = std::vector<float>(heightMapCount);
    for (auto &h : heightMap) {
        h = 0.0F;
    }
    heightBuffer = std::make_shared<VertexBuffer>();
    const BufferLayout heightLayout = {{ShaderDataType::Float, "height"}};
    heightBuffer->setLayout(heightLayout);
    va->addVertexBuffer(heightBuffer);

    const unsigned int trianglesPerQuad = 2;
    const auto indicesCount = WIDTH * LENGTH * trianglesPerQuad;
    auto indices = std::vector<glm::ivec3>(indicesCount);
    unsigned int counter = 0;
    for (unsigned int y = 0; y < LENGTH - 1; y++) {
        for (unsigned int x = 0; x < WIDTH - 1; x++) {
            indices[counter++] = {(y + 1) * WIDTH + x, y * WIDTH + (x + 1), y * WIDTH + x};
            indices[counter++] = {(y + 1) * WIDTH + x, (y + 1) * WIDTH + (x + 1), y * WIDTH + (x + 1)};
        }
    }
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    va->setIndexBuffer(indexBuffer);
}

void AudioVis::renderMesh(const glm::vec3 &modelScale, const bool drawWireframe) {
    shader->bind();
    va->bind();

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, modelScale);
    const auto viewMatrix = getCamera().getViewMatrix();
    const auto projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    shader->setUniform("model", modelMatrix);
    shader->setUniform("view", viewMatrix);
    shader->setUniform("projection", projectionMatrix);

    if (drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    GL_Call(glDrawElements(GL_TRIANGLES, va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    if (drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    va->unbind();
    shader->unbind();
}

void AudioVis::updateMesh(const std::function<double(int)> &calcSample01Func, unsigned int linesPerSecond) {
    RECORD_SCOPE();

    if (linesPerSecond == 0) {
        linesPerSecond = 1;
    }

    for (auto &height : heightMap) {
        height = 0.0F;
    }

    int currentLine = 0;
    int currentCursor = playBack.sampleCursor;
    float maxHeight = 0.0F;

    const int samplesPerLine = (wav.header.sampleRate * wav.header.numChannels) / linesPerSecond;
    while (currentCursor >= std::max(0, playBack.sampleCursor - samplesPerLine * LENGTH)) {
        const auto sample01 = calcSample01Func(currentCursor);
        if (sample01 < 0 || sample01 > 1) {
            std::cout << "Sample is not between 0 and 1: " << sample01 << std::endl;
        }
        const auto bucketIndex = std::floor(sample01 * static_cast<double>(WIDTH));
        const auto index = currentLine * WIDTH + bucketIndex;
        if (index < heightMap.size()) {
            heightMap[index] += 1.0F;
            if (heightMap[index] > maxHeight) {
                maxHeight = heightMap[index];
            }
        }

        if (currentCursor % samplesPerLine == samplesPerLine - 1) {
            currentLine++;
        }

        currentCursor--;
    }

    if (maxHeight > 0.0F) {
        for (auto &height : heightMap) {
            height /= maxHeight;
        }
    }

    heightBuffer->update(heightMap);
}

void AudioVis::updateMeshAmplitude(unsigned int linesPerSecond) {
    updateMesh(
          [this](int currentCursor) {
              const auto sample = static_cast<double>(*(wav.data.data16 + currentCursor));
              const auto min = static_cast<double>(std::numeric_limits<int16_t>::min());
              const auto max = static_cast<double>(std::numeric_limits<int16_t>::max());
              const auto divisor = max - min;
              return (sample - min) / divisor;
          },
          linesPerSecond);
}

void AudioVis::updateMeshPhase(unsigned int linesPerSecond) {
    updateMesh(
          [this](int currentCursor) {
              auto phase = playBack.coefficients[currentCursor].phase;
              phase = std::clamp(phase, -180.0, 180.0);
              return (phase + 180.0F) / 360.0F;
          },
          linesPerSecond);
}

void AudioVis::updateMeshFrequency(unsigned int linesPerSecond) {
    updateMesh(
          [this](int currentCursor) {
              const auto frequency = playBack.coefficients[currentCursor].frequency;
              return frequency / (wav.header.sampleRate * 0.5);
          },
          linesPerSecond);
}

void AudioVis::updateMeshMagnitude(unsigned int linesPerSecond) {
    updateMesh(
          [this](int currentCursor) {
              const auto magnitude = playBack.coefficients[currentCursor].magnitude;
              return magnitude;
          },
          linesPerSecond);
}
