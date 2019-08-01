#pragma once

#include <cstdlib>
#include <functional>

struct Frame {
    unsigned char *buffer = nullptr;
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int channels = 3;
    unsigned int index = 0;
    Frame *next = nullptr;
};

class Video {
public:
    Video() {
        tail = new Frame();
        head = tail;
    }

    void iterateFrames(const std::function<void(Frame *)> &workFunction);

    void recordFrame(unsigned int width, unsigned int height);

    void reset();

    bool hasFrames() {
        return head->buffer != nullptr;
    }

    unsigned int getWidth() const { return head->width; }

    unsigned int getHeight() const { return head->height; }

private:
    Frame *head = nullptr;
    Frame *tail = nullptr;
};

class ScreenRecorder {
public:
    ScreenRecorder() = default;

    void tick(unsigned int windowWidth, unsigned int windowHeight) {
        if (shouldTakeScreenshot) {
            saveScreenshot(windowWidth, windowHeight);
            shouldTakeScreenshot = false;
        }

        if (recording) {
            video.recordFrame(windowWidth, windowHeight);
        }
    }

    void takeScreenshot() {
        shouldTakeScreenshot = true;
    }

    void startRecording() {
        recording = true;
    }

    void stopRecording() {
        recording = false;
        saveRecording();
    }

    bool isRecording() {
        return recording;
    }

    static void saveScreenshot(unsigned int windowWidth, unsigned int windowHeight);

    void saveRecording();

    enum RecordingType {
        GIF = 0,
        PNG
    };

    RecordingType recordingType = GIF;

private:
    bool shouldTakeScreenshot = false;
    bool recording = false;
    unsigned int recordingIndex = 0;
    Video video = {};

    void saveRecordingAsPng();

    void saveRecordingAsGif();
};
