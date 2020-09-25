#include "AudioVis.h"

#include <soundio/soundio.h>

DEFINE_SHADER(audio_vis_AudioVis)

void AudioVis::setup() {
    shader = SHADER(audio_vis_AudioVis);

    loadWavFile("../../src/test/audio_test.wav", wav);
    playBack.wav = &wav;
    initSoundIo(wav.header.sampleRate);
}

void AudioVis::destroy() {
    std::free(wav.data.data8);
    soundio_outstream_destroy(outstream);
    soundio_device_unref(device);
    soundio_destroy(soundio);
}

void AudioVis::tick() {
    ImGui::Begin("Settings");
    ImGui::Text("Sample Rate: %d", wav.header.sampleRate);
    ImGui::Text("Bits per Sample: %d", wav.header.bitsPerSample);
    ImGui::Text("Number of Channels: %d", wav.header.numChannels);

    float seconds = static_cast<float>(playBack.sampleCursor) /
                    (static_cast<float>(wav.header.sampleRate) * static_cast<float>(wav.header.numChannels));
    ImGui::Text("%.2fs", seconds);

    ImGui::SliderInt("", &playBack.sampleCursor, 0, static_cast<int>(wav.data.subChunkSize) / 2, "");

    std::string btnText = "Pause";
    if (playBack.paused) {
        btnText = "Resume";
    }
    if (ImGui::Button(btnText.c_str())) {
        playBack.paused = !playBack.paused;
        soundio_outstream_pause(outstream, playBack.paused);
    }

    ImGui::End();

    soundio_flush_events(soundio);
}

void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max) {
    auto *playBack = reinterpret_cast<PlayBack *>(outstream->userdata);
    const SoundIoChannelLayout *layout = &outstream->layout;
    int frames_left = frame_count_max;
    SoundIoChannelArea *areas = nullptr;

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
            playBack->sampleCursor = playBack->wav->data.subChunkSize / 2;
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
