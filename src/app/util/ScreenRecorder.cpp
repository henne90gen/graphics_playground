#include "ScreenRecorder.h"

#include <sstream>
#include <iostream>
#include <iomanip>
#include <filesystem>

#include "OpenGLUtils.h"
#include "Image.h"

#define GIF_FLIP_VERT

#include <gif.h>
#include <functional>

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
    int width = video.width;
    int height = video.height;

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
    Frame *currentFrame = first;
    do {
        if (currentFrame->buffer == nullptr) {
            continue;
        }

        workFunction(currentFrame);

        Frame *tmp = currentFrame->next;
        currentFrame = tmp;
    } while (currentFrame->next);
}

void Video::recordFrame(unsigned int _width, unsigned int _height) {
    if (width == 0 && height == 0) {
        width = _width;
        height = _height;
    } else if (width != _width || height != _height) {
        std::cerr << "Do not resize the window while recording. Aborted." << std::endl;
        return;
    }
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
    tmp->next = last;
}

void Video::reset() {
    width = 0;
    height = 0;
    Frame *currentFrame = first;
    do {
        if (currentFrame->buffer == nullptr) {
			auto next = currentFrame->next;
			free(currentFrame);
			if (next) {
				continue;
			} else {
	            break;
			}
        }

        Frame *tmp = currentFrame->next;
        free(currentFrame->buffer);
        free(currentFrame);
        currentFrame = tmp;
    } while (currentFrame->next);
	last = new Frame();
	first = last;
}
