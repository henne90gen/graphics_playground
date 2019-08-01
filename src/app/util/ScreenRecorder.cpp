#include <cmath>

#include "ScreenRecorder.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "Image.h"
#include "OpenGLUtils.h"
#include "TimeUtils.h"

#define GIF_FLIP_VERT

#include <functional>
#include <gif.h>

std::string generateScreenshotFilename() {
    std::stringstream buffer;
    std::time_t t = std::time(nullptr);
    buffer << std::put_time(std::localtime(&t), "%Y-%m-%d-%H:%M:%S");
    return "../../../screenshot-" + buffer.str() + ".png";
}

std::string generateScreenrecordingFilename(const std::string &directory, unsigned int frameIndex) {
    return directory + std::to_string(frameIndex) + ".png";
}

std::string generateScreenrecordingDirectoryName(unsigned int recordingIndex) {
    std::stringstream buffer;
    std::time_t t = std::time(nullptr);
    buffer << std::put_time(std::localtime(&t), "%Y-%m-%d-%H-%M-%S");
    return "../../../screenrecording-" + buffer.str() + "-" + std::to_string(recordingIndex) + "/";
}

std::string generateScreenrecordingGifName(unsigned int recordingIndex) {
    std::string dir = generateScreenrecordingDirectoryName(recordingIndex);
    std::string fileName = dir.substr(0, dir.size() - 1);
    return fileName + ".gif";
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

void saveFrameToImage(Frame *frame, const std::string &directory) {
    Image image = {};
    image.fileName = generateScreenrecordingFilename(directory, frame->index);
    image.width = frame->width;
    image.height = frame->height;
    image.channels = frame->channels;

    const int numberOfPixels = frame->width * frame->height * frame->channels;
    image.pixels = std::vector<unsigned char>(numberOfPixels);

    unsigned char *ptr = frame->buffer;
    for (unsigned char &pixel : image.pixels) {
        pixel = *ptr;
        ptr++;
    }

    ImageOps::save(image);
}

void ScreenRecorder::saveRecordingAsPng() {
    std::string directory = generateScreenrecordingDirectoryName(recordingIndex);
    if (!std::filesystem::exists(directory)) {
        std::filesystem::create_directory(directory);
    }

    std::function<void(Frame *)> workFunction = [&directory](Frame *currentFrame) {
        saveFrameToImage(currentFrame, directory);
    };
    video.iterateFrames(workFunction);
}

void ScreenRecorder::saveRecordingAsGif() {
    if (!video.hasFrames()) {
        std::cerr << "There are no frames in this video." << std::endl;
        return;
    }
    int width = video.getWidth();
    int height = video.getHeight();

    auto fileName = generateScreenrecordingGifName(recordingIndex);
    int delay = 1;
    GifWriter g = {};
    if (!GifBegin(&g, fileName.c_str(), width, height, delay)) {
        std::cerr << "Could not open " << fileName << " for writing." << std::endl;
        video.reset();
        return;
    }

    std::function<void(Frame *)> workFunction = [&g, &delay](Frame *currentFrame) {
        GifWriteFrame(&g, currentFrame->buffer, currentFrame->width, currentFrame->height, delay);
    };
    video.iterateFrames(workFunction);

    if (!GifEnd(&g)) {
        std::cerr << "Could not save to " << fileName << std::endl;
    }
}

void ScreenRecorder::saveRecording() {
    if (recordingType == RecordingType::PNG) {
        saveRecordingAsPng();
    } else if (recordingType == RecordingType::GIF) {
        saveRecordingAsGif();
    } else {
        std::cerr << "Recording type is not supported (" << recordingType << ")" << std::endl;
    }
    recordingIndex++;
    video.reset();
}

void Video::iterateFrames(const std::function<void(Frame *)> &workFunction) {
    Frame *currentFrame = head;
    while (currentFrame != nullptr) {
        if (currentFrame->buffer != nullptr) {
            // only work on frame, if the buffer points to valid memory
            workFunction(currentFrame);
        }

        currentFrame = currentFrame->next;
    }
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

void copyAndScaleDownFrame(const unsigned char *buffer, Frame *frame, unsigned int screenWidth, unsigned int screenHeight) {
    for (unsigned int y = 0; y < frame->height; y++) {
        for (unsigned int x = 0; x < frame->width; x++) {
            auto xF = (float) x;
            auto yF = (float) y;
            float downScaledX = xF / (float) frame->width;
            float downScaledY = yF / (float) frame->height;
            float upScaledX = downScaledX * (float) screenWidth;
            float upScaledY = downScaledY * (float) screenHeight;

            unsigned int xLeft = std::floor(upScaledX);
            unsigned int xRight = std::ceil(upScaledX);
            unsigned int yTop = std::floor(upScaledY);
            unsigned int yBottom = std::ceil(upScaledY);

            unsigned int index = (xLeft + yTop * screenWidth) * frame->channels;
            unsigned int topLeft = *(unsigned int *) (buffer + index);
            index = (xRight + yTop * screenWidth) * frame->channels;
            unsigned int topRight = *(unsigned int *) (buffer + index);

            index = (xLeft + yBottom * screenWidth) * frame->channels;
            unsigned int bottomLeft = *(unsigned int *) (buffer + index);
            index = (xRight + yBottom * screenWidth) * frame->channels;
            unsigned int bottomRight = *(unsigned int *) (buffer + index);

            unsigned int top;
            averagePixel(topLeft, topRight, &top);
            unsigned int bottom;
            averagePixel(bottomLeft, bottomRight, &bottom);

            unsigned int baseIndex = (x + y * frame->width) * frame->channels;
            auto *pixel = (unsigned int *) (frame->buffer + baseIndex);
            averagePixel(top, bottom, pixel);
        }
    }
}

void Video::recordFrame(unsigned int screenWidth, unsigned int screenHeight) {
    const unsigned int channels = 4;
    const int numberOfPixels = screenWidth * screenHeight * channels;
    auto *buffer = static_cast<unsigned char *>(malloc(numberOfPixels * sizeof(unsigned char)));

    GL_Call(glPixelStorei(GL_PACK_ALIGNMENT, 1));
    GL_Call(glReadBuffer(GL_FRONT));
    GL_Call(glReadPixels(0, 0, screenWidth, screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer));

    tail->width = 800;
    tail->height = 600;
    tail->channels = channels;
    tail->buffer = static_cast<unsigned char *>(malloc(
            tail->width * tail->height * tail->channels * sizeof(unsigned char)));

    copyAndScaleDownFrame(buffer, tail, screenWidth, screenHeight);

    Frame *tmp = tail;
    tail = new Frame();
    tail->index = tmp->index + 1;
    tmp->next = tail;
}

void Video::reset() {
    Frame *currentFrame = head;
    while (currentFrame != nullptr) {
        if (currentFrame->buffer == nullptr) {
            auto next = currentFrame->next;
            free(currentFrame);
            if (next != nullptr) {
                continue;
            }
            break;

        }

        Frame *tmp = currentFrame->next;
        free(currentFrame->buffer);
        free(currentFrame);
        currentFrame = tmp;
    }
    tail = new Frame();
    head = tail;
}
