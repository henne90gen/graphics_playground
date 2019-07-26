#include "ScreenRecorder.h"

#include <sstream>
#include <iostream>
#include <iomanip>
#include <filesystem>

#include "OpenGLUtils.h"
#include "Image.h"

#define GIF_FLIP_VERT

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
    buffer << std::put_time(std::localtime(&t), "%Y-%m-%d-%H:%M:%S");
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

void ScreenRecorder::saveRecording() {
    // FIXME refactor this method (split gif and png saving into two separate methods)
    Frame *currentFrame = last->previous;

    int width = currentFrame->width;
    int height = currentFrame->height;
    std::vector<uint8_t> black(width * height * 4, 0);
    std::vector<uint8_t> white(width * height * 4, 255);

    std::string directory = generateScreenrecordingDirectoryName(recordingIndex);
    if (recordingType == RecordingType::PNG && !std::filesystem::exists(directory)) {
        std::filesystem::create_directory(directory);
    }

    auto fileName = generateScreenrecordingGifName(recordingIndex);
    int delay = 1;
    GifWriter g = {};
    if (recordingType == RecordingType::GIF) {
        GifBegin(&g, fileName.c_str(), width, height, delay);
    }

    do {
        if (currentFrame->buffer == nullptr) {
            free(currentFrame);
            break;
        }

        if (recordingType == RecordingType::GIF) {
            // FIXME go through all the frames in the correct order, otherwise the gif will play back in reverse
            GifWriteFrame(&g, currentFrame->buffer, currentFrame->width, currentFrame->height, delay);
        } else if (recordingType == RecordingType::PNG) {
            saveFrameToImage(currentFrame, directory);
        }

        Frame *tmp = currentFrame->previous;
        free(currentFrame->buffer);
        free(currentFrame);
        currentFrame = tmp;
    } while (currentFrame->previous);

    if (recordingType == RecordingType::GIF) {
        GifEnd(&g);
    }

    last = new Frame();
    recordingIndex++;
}

void ScreenRecorder::recordFrame(unsigned int width, unsigned int height) {
    last->width = width;
    last->height = height;
    last->channels = 4;
    const int numberOfPixels = width * height * last->channels;
    last->buffer = (unsigned char *) malloc(numberOfPixels * sizeof(unsigned char));

    GL_Call(glPixelStorei(GL_PACK_ALIGNMENT, 1));
    GL_Call(glReadBuffer(GL_FRONT));
    GL_Call(glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, last->buffer));

    Frame *tmp = last;
    last = new Frame();
    last->index = tmp->index + 1;
    last->previous = tmp;
}
