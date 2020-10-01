#include "AudioVis.h"

#include <soundio/soundio.h>

constexpr float FIELD_OF_VIEW = 45.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 100000.0F;

constexpr auto WIDTH = 50;
constexpr auto LENGTH = 100;

DEFINE_SHADER(audio_vis_AudioVis)

void AudioVis::setup() {
    shader = SHADER(audio_vis_AudioVis);

    loadWavFile("../../src/test/audio_test.wav", wav);
    playBack.wav = &wav;
    auto inputData = std::vector<float>(wav.data.subChunkSize);
    for (unsigned int i = 0; i < wav.data.subChunkSize; i++) {
        inputData[i] = static_cast<float>(wav.data.data8[i]);
    }
    playBack.coefficients = fourier::fft(inputData, playBack.wav->header.sampleRate);
    initSoundIo(wav.header.sampleRate);

#if 1
    playBack.paused = true;
    soundio_outstream_pause(outstream, playBack.paused);
#endif

    initMesh();
}

void AudioVis::destroy() {
    std::free(wav.data.data8);
    soundio_outstream_destroy(outstream);
    soundio_device_unref(device);
    soundio_destroy(soundio);
}

void AudioVis::tick() {
    static auto modelScale = glm::vec3(1.0F, 20.0F, -1.0F);
    static auto cameraPosition = glm::vec3(-35.0F, -75.0F, -50.0F);
    static glm::vec3 cameraRotation = {0.75F, -0.25F, 0.0F};
    static bool drawWireframe = true;
    static int linesPerSecond = 15;
    static VisMode currentMode = VisMode::AMPLITUDE;

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), 0.001F);
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition), 0.1F);
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), 0.001F);
    ImGui::Checkbox("Wireframe", &drawWireframe);

    ImGui::Separator();

    ImGui::Text("Sample Rate: %d", wav.header.sampleRate);
    ImGui::Text("Bits per Sample: %d", wav.header.bitsPerSample);
    ImGui::Text("Number of Channels: %d", wav.header.numChannels);

    float seconds = static_cast<float>(playBack.sampleCursor) /
                    (static_cast<float>(wav.header.sampleRate) * static_cast<float>(wav.header.numChannels));
    ImGui::Text("%.2fs", seconds);

    ImGui::SliderInt("", &playBack.sampleCursor, 0, static_cast<int>(wav.data.subChunkSize) / wav.header.numChannels,
                     "");

    std::string btnText = "Pause";
    if (playBack.paused) {
        btnText = "Resume";
    }
    if (ImGui::Button(btnText.c_str())) {
        playBack.paused = !playBack.paused;
        soundio_outstream_pause(outstream, playBack.paused);
    }

    ImGui::DragInt("Lines Per Second", &linesPerSecond, 1, 1, wav.header.sampleRate);
    // TODO make this a dropdown
    ImGui::DragInt("Current Mode", reinterpret_cast<int *>(&currentMode));
    ImGui::End();

    soundio_flush_events(soundio);

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
    renderMesh(modelScale, cameraPosition, cameraRotation, drawWireframe);
}

void writeCallback(struct SoundIoOutStream *outstream, int frameCountMin, int frameCountMax) {
    auto *playBack = reinterpret_cast<PlayBack *>(outstream->userdata);
    const SoundIoChannelLayout *layout = &outstream->layout;
    SoundIoChannelArea *areas = nullptr;

    int framesLeft = frameCountMax;
    while (framesLeft > 0) {
        int frameCount = framesLeft;

        int err = soundio_outstream_begin_write(outstream, &areas, &frameCount);
        if (err != 0) {
            fprintf(stderr, "%s\n", soundio_strerror(err));
            exit(1);
        }

        if (frameCount == 0) {
            break;
        }

        bool playBackEnded = false;
        for (int frame = 0; frame < frameCount; frame++) {
            for (int channel = 0; channel < layout->channel_count; channel++) {
                int sampleOffset = playBack->sampleCursor + (layout->channel_count * frame + channel);
                unsigned int sampleOffsetBytes = sampleOffset * 2;
                if (sampleOffsetBytes >= playBack->wav->data.subChunkSize) {
                    playBackEnded = true;
                    break;
                }
                int16_t *samplePtr = playBack->wav->data.data16 + sampleOffset;
                auto *ptr = (int16_t *)(areas[channel].ptr + areas[channel].step * frame);
                *ptr = *samplePtr;
            }
            if (playBackEnded) {
                break;
            }
        }
        playBack->sampleCursor += frameCount * layout->channel_count;

        err = soundio_outstream_end_write(outstream);
        if (err != 0) {
            fprintf(stderr, "%s\n", soundio_strerror(err));
            exit(1);
        }

        if (playBackEnded) {
            playBack->sampleCursor = playBack->wav->data.subChunkSize / playBack->wav->header.numChannels;
            playBack->paused = true;
            err = soundio_outstream_pause(outstream, true);
            if (err != 0) {
                fprintf(stderr, "%s\n", soundio_strerror(err));
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
        fprintf(stderr, "out of memory\n");
        return;
    }

    int err = soundio_connect(soundio);
    if (err != 0) {
        fprintf(stderr, "error connecting: %s\n", soundio_strerror(err));
        return;
    }

    soundio_flush_events(soundio);

    int default_out_device_index = soundio_default_output_device_index(soundio);
    if (default_out_device_index < 0) {
        fprintf(stderr, "no output device found\n");
        return;
    }

    device = soundio_get_output_device(soundio, default_out_device_index);
    if (device == nullptr) {
        fprintf(stderr, "out of memory\n");
        return;
    }

    fprintf(stderr, "Output device: %s\n", device->name);

    outstream = soundio_outstream_create(device);
    if (outstream == nullptr) {
        fprintf(stderr, "out of memory\n");
        return;
    }
    outstream->format = SoundIoFormatS16LE;
    outstream->sample_rate = sampleRate;
    outstream->userdata = &playBack;
    outstream->write_callback = writeCallback;

    err = soundio_outstream_open(outstream);
    if (err != 0) {
        fprintf(stderr, "unable to open device: %s", soundio_strerror(err));
        return;
    }

    if (outstream->layout_error != 0) {
        fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(outstream->layout_error));
    }

    err = soundio_outstream_start(outstream);
    if (err != 0) {
        fprintf(stderr, "unable to start device: %s\n", soundio_strerror(err));
        return;
    }
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
    BufferLayout positionLayout = {{ShaderDataType::Float2, "position"}};
    auto positionBuffer = std::make_shared<VertexBuffer>(vertices.data(), verticesSize, positionLayout);
    va->addVertexBuffer(positionBuffer);

    const unsigned int heightMapCount = WIDTH * LENGTH;
    heightMap = std::vector<float>(heightMapCount);
    for (auto &h : heightMap) {
        h = 0.0F;
    }
    heightBuffer = std::make_shared<VertexBuffer>();
    BufferLayout heightLayout = {{ShaderDataType::Float, "height"}};
    heightBuffer->setLayout(heightLayout);
    va->addVertexBuffer(heightBuffer);

    const unsigned int trianglesPerQuad = 2;
    unsigned int indicesCount = WIDTH * LENGTH * trianglesPerQuad;
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

void AudioVis::renderMesh(const glm::vec3 &modelScale, const glm::vec3 &cameraPosition, const glm::vec3 &cameraRotation,
                          const bool drawWireframe) {
    shader->bind();
    va->bind();

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, modelScale);
    glm::mat4 viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
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
    if (linesPerSecond == 0) {
        linesPerSecond = 1;
    }

    for (auto &height : heightMap) {
        height = 0.0F;
    }

    int currentLine = 0;
    int currentCursor = playBack.sampleCursor;
    float maxHeight = 0.0F;
    unsigned int samplesPerLine = (wav.header.sampleRate * wav.header.numChannels) / linesPerSecond;
    while (currentCursor >= 0) {
        double sample01 = calcSample01Func(currentCursor);
        unsigned long bucketIndex = std::floor(sample01 * static_cast<double>(WIDTH));
        unsigned long index = currentLine * WIDTH + bucketIndex;
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
              auto sample = static_cast<double>(*(wav.data.data16 + currentCursor));
              double min = std::abs(static_cast<double>(std::numeric_limits<int16_t>::min()));
              double max = std::abs(static_cast<double>(std::numeric_limits<int16_t>::max()));
              return (sample + min) / (min + max);
          },
          linesPerSecond);
}

void AudioVis::updateMeshPhase(unsigned int linesPerSecond) {
    updateMesh(
          [this](int currentCursor) {
              float phase = playBack.coefficients[currentCursor].phase;
              return (phase + 180.0F) / 360.0F;
          },
          linesPerSecond);
}

void AudioVis::updateMeshFrequency(unsigned int linesPerSecond) {
    updateMesh(
          [this](int currentCursor) {
              double frequency = playBack.coefficients[currentCursor].frequency;
              return frequency / (wav.header.sampleRate * 0.5);
          },
          linesPerSecond);
}

void AudioVis::updateMeshMagnitude(unsigned int linesPerSecond) {
    updateMesh(
          [this](int currentCursor) {
              float magnitude = playBack.coefficients[currentCursor].magnitude;
              return magnitude;
          },
          linesPerSecond);
}
