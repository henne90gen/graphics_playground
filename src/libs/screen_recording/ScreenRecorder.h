#pragma once

#include <VideoSaver.h>

class ScreenRecorder {
  public:
    ScreenRecorder() = default;

    void tick(unsigned int windowWidth, unsigned int windowHeight);

    void takeScreenshot() { shouldTakeScreenshot = true; }
    static void saveScreenshot(unsigned int windowWidth, unsigned int windowHeight);

    void startRecording();
    void stopRecording();
    bool isRecording() { return videoSaver != nullptr; }

    enum RecordingType {
        GIF = 0,
        MP4 = 1,
    };

    RecordingType recordingType = GIF;

  private:
    bool shouldTakeScreenshot = false;
    unsigned int recordingIndex = 0;
    std::unique_ptr<VideoSaver> videoSaver = nullptr;
};
