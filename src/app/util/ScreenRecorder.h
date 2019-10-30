#pragma once

#include "VideoSaver.h"

class ScreenRecorder {
  public:
    ScreenRecorder() = default;

    void tick(unsigned int windowWidth, unsigned int windowHeight);

    void takeScreenshot() { shouldTakeScreenshot = true; }

    void startRecording();

    void stopRecording();

    bool isRecording() { return videoSaver != nullptr; }

    static void saveScreenshot(unsigned int windowWidth, unsigned int windowHeight);

    enum RecordingType {
        GIF = 0,
        MP4,
    };

    RecordingType recordingType = GIF;

  private:
    bool shouldTakeScreenshot = false;
    unsigned int recordingIndex = 0;
    std::unique_ptr<VideoSaver> videoSaver = nullptr;

    void saveRecordingAsGif();
};
