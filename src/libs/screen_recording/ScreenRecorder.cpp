#include "ScreenRecorder.h"

#include <cmath>
#include <functional>
#include <iomanip>
#include <sstream>
#include <iostream>

#include <Image.h>
#include <ImageOps.h>
#include <util/OpenGLUtils.h>

std::string generateScreenshotFilename() {
    std::stringstream buffer;
    std::time_t t = std::time(nullptr);
    buffer << std::put_time(std::localtime(&t), "%Y-%m-%d-%H-%M-%S");
    return "../../screenshot-" + buffer.str() + ".png";
}

std::string generateScreenrecordingDirectoryName(unsigned int recordingIndex) {
    std::stringstream buffer;
    std::time_t t = std::time(nullptr);
    buffer << std::put_time(std::localtime(&t), "%Y-%m-%d-%H-%M-%S");
    return "../../screenrecording-" + buffer.str() + "-" + std::to_string(recordingIndex) + "/";
}

std::string generateScreenrecordingName(unsigned int recordingIndex, ScreenRecorder::RecordingType recordingType) {
    std::string fileExtension;
    switch (recordingType) {
    case ScreenRecorder::GIF:
        fileExtension = ".gif";
        break;
    case ScreenRecorder::MP4:
        fileExtension = ".mp4";
        break;
    }
    std::string dir = generateScreenrecordingDirectoryName(recordingIndex);
    std::string fileName = dir.substr(0, dir.size() - 1);
    return fileName + fileExtension;
}

void ScreenRecorder::saveScreenshot(unsigned int windowWidth, unsigned int windowHeight) {
    Image image = {};
    image.fileName = generateScreenshotFilename();
    image.width = windowWidth;
    image.height = windowHeight;
    image.channels = 3;

    const unsigned int numberOfPixels = image.width * image.height * image.channels;
    image.pixels = std::vector<uint8_t>(numberOfPixels);

    GL_Call(glPixelStorei(GL_PACK_ALIGNMENT, 1));
    GL_Call(glReadBuffer(GL_FRONT));
    GL_Call(glReadPixels(0, 0, image.width, image.height, GL_RGB, GL_UNSIGNED_BYTE, image.pixels.data()));

    ImageOps::save(image);
    std::cout << "Saved screenshot " << image.fileName << std::endl;
}

void ScreenRecorder::tick(unsigned int windowWidth, unsigned int windowHeight) {
    if (shouldTakeScreenshot) {
        saveScreenshot(windowWidth, windowHeight);
        shouldTakeScreenshot = false;
    }

    if (isRecording()) {
        videoSaver->captureFrame(windowWidth, windowHeight);
    }
}

void ScreenRecorder::stopRecording() {
    if (videoSaver == nullptr) {
        std::cout << "There is no video to save." << std::endl;
        return;
    }
    videoSaver->save();
    videoSaver = nullptr;
    recordingIndex++;
}

void ScreenRecorder::startRecording() {
    std::string fileName = generateScreenrecordingName(recordingIndex, recordingType);
    if (recordingType == RecordingType::GIF) {
        videoSaver = std::make_unique<GifVideoSaver>(fileName);
#if FFMPEG_FOUND
    } else if (recordingType == RecordingType::MP4) {
        videoSaver = std::make_unique<Mp4VideoSaver>(fileName);
#endif
    } else {
        std::cerr << "Recording type is not supported (" << recordingType << ")" << std::endl;
    }
}
