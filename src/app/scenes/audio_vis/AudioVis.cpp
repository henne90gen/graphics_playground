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
    ImGui::End();

    soundio_flush_events(soundio);

    updateMeshAmplitude(linesPerSecond);
    renderMesh(modelScale, cameraPosition, cameraRotation, drawWireframe);
}

void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max) {
    auto *playBack = reinterpret_cast<PlayBack *>(outstream->userdata);
    const SoundIoChannelLayout *layout = &outstream->layout;
    SoundIoChannelArea *areas = nullptr;

    int frames_left = frame_count_max;
    while (frames_left > 0) {
        int frame_count = frames_left;

        int err = soundio_outstream_begin_write(outstream, &areas, &frame_count);
        if (err != 0) {
            fprintf(stderr, "%s\n", soundio_strerror(err));
            exit(1);
        }

        if (frame_count == 0) {
            break;
        }

        bool playBackEnded = false;
        for (int frame = 0; frame < frame_count; frame++) {
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
        playBack->sampleCursor += frame_count * layout->channel_count;

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

        frames_left -= frame_count;
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
    outstream->write_callback = write_callback;

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

void AudioVis::updateMeshAmplitude(unsigned int linesPerSecond) {
    if (linesPerSecond == 0) {
        linesPerSecond = 1;
    }

    for (auto &height : heightMap) {
        height = 0.0F;
    }

    int currentCursor = playBack.sampleCursor;
    int currentLine = 0;
    float maxHeight = 0.0F;
    unsigned int samplesPerBucket = (wav.header.sampleRate * wav.header.numChannels) / linesPerSecond;
    while (currentCursor >= 0) {
        int32_t sample = *(wav.data.data16 + currentCursor);
        float sample01 = static_cast<float>(sample - std::numeric_limits<int16_t>::min()) /
                         (static_cast<float>(std::numeric_limits<int16_t>::min()) * -1.0F +
                          static_cast<float>(std::numeric_limits<int16_t>::max()));
        unsigned long bucketIndex = std::floor(sample01 * static_cast<float>(WIDTH));
        unsigned long index = currentLine * WIDTH + bucketIndex;
        if (index < heightMap.size()) {
            heightMap[index] += 1.0F;
            if (heightMap[index] > maxHeight) {
                maxHeight = heightMap[index];
            }
        }

        if (currentCursor % samplesPerBucket == samplesPerBucket - 1) {
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

void AudioVis::updateMeshFrequency() {}
