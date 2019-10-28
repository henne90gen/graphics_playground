#include "Video.h"

#include "OpenGLUtils.h"

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

void Video::recordFrame(unsigned int screenWidth, unsigned int screenHeight) {
    const unsigned int channels = 4;
    tail->width = screenWidth;
    tail->height = screenHeight;
    tail->channels = channels;
    tail->buffer =
          static_cast<unsigned char *>(malloc(tail->width * tail->height * tail->channels * sizeof(unsigned char)));

    GL_Call(glPixelStorei(GL_PACK_ALIGNMENT, 1));
    GL_Call(glReadBuffer(GL_FRONT));
    GL_Call(glReadPixels(0, 0, screenWidth, screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, tail->buffer));

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
