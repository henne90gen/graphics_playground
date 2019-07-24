#pragma once

#include <cstdlib>

#include "OpenGLUtils.h"

struct Frame {
    unsigned char *buffer;
    Frame *previous;
};

class ScreenRecorder {
public:
    ScreenRecorder() {
        last = new Frame();
    }

    void recordFrame() {
        last->buffer = (unsigned char *) malloc(1);

    }

    void tick(unsigned int windowWidth, unsigned int windowHeight) {
        if (shouldTakeScreenshot) {
            saveScreenshot(windowWidth, windowHeight);
            shouldTakeScreenshot = false;
        }

    }

private:
    bool shouldTakeScreenshot = false;
    bool isRecording = false;
    Frame *last;
};



