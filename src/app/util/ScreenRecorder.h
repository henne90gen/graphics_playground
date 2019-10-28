#pragma once

#include "Video.h"

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

    void takeScreenshot() { shouldTakeScreenshot = true; }

    void startRecording() { recording = true; }

    void stopRecording() {
        recording = false;
        saveRecording();
    }

    bool isRecording() { return recording; }

    static void saveScreenshot(unsigned int windowWidth, unsigned int windowHeight);

    void saveRecording();

    enum RecordingType {
        GIF = 0,
        PNG,
        MP4,
    };

    RecordingType recordingType = GIF;

  private:
    bool shouldTakeScreenshot = false;
    bool recording = false;
    unsigned int recordingIndex = 0;
    Video video = {};

    void saveRecordingAsPng();

    void saveRecordingAsGif();

    void saveRecordingAsMp4();
};
