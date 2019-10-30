#include <cmath>

#include "ScreenRecorder.h"

#include <filesystem>
#include <functional>
#include <iomanip>
#include <iostream>

#define GIF_FLIP_VERT
#include <gif.h>

#include "Image.h"
#include "OpenGLUtils.h"

std::string generateScreenshotFilename() {
    std::stringstream buffer;
    std::time_t t = std::time(nullptr);
    buffer << std::put_time(std::localtime(&t), "%Y-%m-%d-%H:%M:%S");
    return "../../../screenshot-" + buffer.str() + ".png";
}

std::string generateScreenrecordingDirectoryName(unsigned int recordingIndex) {
    std::stringstream buffer;
    std::time_t t = std::time(nullptr);
    buffer << std::put_time(std::localtime(&t), "%Y-%m-%d-%H-%M-%S");
    return "../../../screenrecording-" + buffer.str() + "-" + std::to_string(recordingIndex) + "/";
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

    const int numberOfPixels = image.width * image.height * image.channels;
    image.pixels = std::vector<unsigned char>(numberOfPixels);

    GL_Call(glPixelStorei(GL_PACK_ALIGNMENT, 1));
    GL_Call(glReadBuffer(GL_FRONT));
    GL_Call(glReadPixels(0, 0, image.width, image.height, GL_RGB, GL_UNSIGNED_BYTE, image.pixels.data()));

    ImageOps::save(image);
    std::cout << "Saved screenshot " << image.fileName << std::endl;
}

void averagePixel(unsigned int pixel1, unsigned int pixel2, unsigned int *dest) {
    unsigned int r1 = (pixel1 & 0x000000ff);
    unsigned int g1 = (pixel1 & 0x0000ff00) >> 8;
    unsigned int b1 = (pixel1 & 0x00ff0000) >> 16;
    unsigned int r2 = (pixel2 & 0x000000ff);
    unsigned int g2 = (pixel2 & 0x0000ff00) >> 8;
    unsigned int b2 = (pixel2 & 0x00ff0000) >> 16;
    unsigned int r = (r1 + r2) / 2;
    unsigned int g = (g1 + g2) / 2;
    unsigned int b = (b1 + b2) / 2;
    *dest = r + (g << 8) + (b << 16);
}

void scaleDownFrame(Frame *frame, const unsigned int newWidth = 800, const unsigned int newHeight = 600) {
    auto buffer = static_cast<unsigned char *>(malloc(newWidth * newHeight * frame->channels * sizeof(unsigned char)));

    const unsigned int oldWidth = frame->width;
    const unsigned int oldHeight = frame->height;

    frame->width = newWidth;
    frame->height = newHeight;

    for (unsigned int y = 0; y < frame->height; y++) {
        for (unsigned int x = 0; x < frame->width; x++) {
            auto xF = (float)x;
            auto yF = (float)y;
            float downScaledX = xF / (float)frame->width;
            float downScaledY = yF / (float)frame->height;
            float upScaledX = downScaledX * (float)oldWidth;
            float upScaledY = downScaledY * (float)oldHeight;

            unsigned int xLeft = std::floor(upScaledX);
            unsigned int xRight = std::ceil(upScaledX);
            unsigned int yTop = std::floor(upScaledY);
            unsigned int yBottom = std::ceil(upScaledY);

            unsigned int index = (xLeft + yTop * oldWidth) * frame->channels;
            unsigned int topLeft = *(unsigned int *)(frame->buffer + index);
            index = (xRight + yTop * oldWidth) * frame->channels;
            unsigned int topRight = *(unsigned int *)(frame->buffer + index);

            index = (xLeft + yBottom * oldWidth) * frame->channels;
            unsigned int bottomLeft = *(unsigned int *)(frame->buffer + index);
            index = (xRight + yBottom * oldWidth) * frame->channels;
            unsigned int bottomRight = *(unsigned int *)(frame->buffer + index);

            unsigned int top;
            averagePixel(topLeft, topRight, &top);
            unsigned int bottom;
            averagePixel(bottomLeft, bottomRight, &bottom);

            unsigned int baseIndex = (x + y * frame->width) * frame->channels;
            auto *pixel = (unsigned int *)(buffer + baseIndex);
            averagePixel(top, bottom, pixel);
        }
    }

    free(frame->buffer);
    frame->buffer = buffer;
}

void ScreenRecorder::saveRecordingAsGif() {
#if 0
    int width = (int)video.getWidth();
    int height = (int)video.getHeight();

    auto fileName = generateScreenrecordingGifName(recordingIndex);
    int delay = 1;
    GifWriter g = {};
    if (!GifBegin(&g, fileName.c_str(), width, height, delay)) {
        std::cerr << "Could not open " << fileName << " for writing." << std::endl;
        video.reset();
        return;
    }

    std::function<void(Frame *)> workFunction = [&g, &delay](Frame *currentFrame) {
        scaleDownFrame(currentFrame);
        GifWriteFrame(&g, currentFrame->buffer, currentFrame->width, currentFrame->height, delay);
    };
    video.iterateFrames(workFunction);

    if (!GifEnd(&g)) {
        std::cerr << "Could not save to " << fileName << std::endl;
    }
#endif
}

std::unique_ptr<Frame> captureFrame(unsigned int screenWidth, unsigned int screenHeight) {
    const unsigned int channels = 4;
    std::unique_ptr<Frame> frame = std::make_unique<Frame>();
    frame->width = screenWidth;
    frame->height = screenHeight;
    frame->channels = channels;
    const unsigned int numberOfPixels = frame->width * frame->height;
    frame->buffer = static_cast<unsigned char *>(malloc(numberOfPixels * frame->channels * sizeof(unsigned char)));

    GL_Call(glPixelStorei(GL_PACK_ALIGNMENT, 1));
    GL_Call(glReadBuffer(GL_FRONT));
    GL_Call(glReadPixels(0, 0, frame->width, frame->height, GL_RGBA, GL_UNSIGNED_BYTE, frame->buffer));

    return frame;
}

void ScreenRecorder::tick(unsigned int windowWidth, unsigned int windowHeight) {
    if (shouldTakeScreenshot) {
        saveScreenshot(windowWidth, windowHeight);
        shouldTakeScreenshot = false;
    }

    if (isRecording()) {
        std::unique_ptr<Frame> frame = captureFrame(windowWidth, windowHeight);
        videoSaver->acceptFrame(frame);
    }
}

void ScreenRecorder::stopRecording() {
    // FIXME refactor the ScreenRecorder to give the frames directly to the underlying saving mechanism, maybe with an
    //       asynchronous queue or something
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
        //        videoSaver = std::make_unique<GifVideoSaver>(fileName);
    } else if (recordingType == RecordingType::MP4) {
        videoSaver = std::make_unique<Mp4VideoSaver>(fileName);
    } else {
        std::cerr << "Recording type is not supported (" << recordingType << ")" << std::endl;
    }
}
