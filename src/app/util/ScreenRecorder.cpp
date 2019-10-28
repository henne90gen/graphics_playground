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
#include "VideoSaver.h"

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

std::string generateScreenrecordingName(unsigned int recordingIndex, std::string fileExtension) {
    std::string dir = generateScreenrecordingDirectoryName(recordingIndex);
    std::string fileName = dir.substr(0, dir.size() - 1);
    return fileName + fileExtension;
}

std::string generateScreenrecordingGifName(unsigned int recordingIndex) {
    return generateScreenrecordingName(recordingIndex, ".gif");
}

std::string generateScreenrecordingMp4Name(unsigned int recordingIndex) {
    return generateScreenrecordingName(recordingIndex, ".mp4");
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
}

void ScreenRecorder::saveRecordingAsMp4() {
    auto videoFileName = generateScreenrecordingMp4Name(recordingIndex);
    auto videoSaver = VideoSaver(&video, videoFileName);
    videoSaver.run();
}

void ScreenRecorder::saveRecording() {
    if (!video.hasFrames()) {
        std::cerr << "There are no frames in this recording." << std::endl;
        return;
    }
    if (recordingType == RecordingType::GIF) {
        saveRecordingAsGif();
    } else if (recordingType == RecordingType::MP4) {
        saveRecordingAsMp4();
    } else {
        std::cerr << "Recording type is not supported (" << recordingType << ")" << std::endl;
    }
    recordingIndex++;
    video.reset();
}
