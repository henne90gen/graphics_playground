#pragma once

#include <cstdlib>

struct Frame {
    unsigned char *buffer = nullptr;
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int channels = 3;
    unsigned int index = 0;
    Frame *previous = nullptr;
};

class ScreenRecorder {
public:
    ScreenRecorder() {
        last = new Frame();
    }

    void tick(unsigned int windowWidth, unsigned int windowHeight) {
        if (shouldTakeScreenshot) {
            saveScreenshot(windowWidth, windowHeight);
            shouldTakeScreenshot = false;
        }

        if (recording) {
            recordFrame(windowWidth, windowHeight);
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

    void recordFrame(unsigned int width, unsigned int height);

    void saveRecording();

    enum RecordingType {
        GIF = 0,
        PNG
    };

    RecordingType recordingType = GIF;

private:
    bool shouldTakeScreenshot = false;
    bool recording = false;
    Frame *last;
    unsigned int recordingIndex = 0;
};
