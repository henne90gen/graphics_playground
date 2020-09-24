#include "AudioVis.h"

#include <soundio/soundio.h>

DEFINE_SHADER(audio_vis_AudioVis)

void AudioVis::setup() {
    shader = SHADER(audio_vis_AudioVis);

    loadWavFile("../../src/test/audio_test.wav", wav);
    initSoundIo();
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
    ImGui::End();

    soundio_flush_events(soundio);
}

static const float PI = 3.1415926535f;
static float seconds_offset = 0.0F;
static void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max) {
    const SoundIoChannelLayout *layout = &outstream->layout;
    const auto float_sample_rate = static_cast<float>(outstream->sample_rate);
    const auto seconds_per_frame = 1.0F / float_sample_rate;
    auto frames_left = frame_count_max;
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

        const float pitch = 440.0F;
        const float radians_per_second = pitch * 2.0F * PI;
        for (int frame = 0; frame < frame_count; frame += 1) {
            const float sample =
                  sin((seconds_offset + static_cast<float>(frame) * seconds_per_frame) * radians_per_second);
            for (int channel = 0; channel < layout->channel_count; channel += 1) {
                auto *areasPtr = areas[channel].ptr;
                auto *ptr = reinterpret_cast<float *>(areasPtr[areas[channel].step * frame]);
                *ptr = sample;
            }
        }
        seconds_offset = fmod(seconds_offset + seconds_per_frame * static_cast<float>(frame_count), 1.0);

        err = soundio_outstream_end_write(outstream);
        if (err != 0) {
            fprintf(stderr, "%s\n", soundio_strerror(err));
            exit(1);
        }

        frames_left -= frame_count;
    }
}

void AudioVis::initSoundIo() {
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
    outstream->format = SoundIoFormatFloat32NE;
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
