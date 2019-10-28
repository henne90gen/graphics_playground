#pragma once

#include <functional>

struct Frame {
    unsigned char *buffer = nullptr;
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int channels = 3;
    unsigned int index = 0;
    Frame *next = nullptr;
};

class Video {
  public:
    Video() {
        tail = new Frame();
        head = tail;
    }

    void iterateFrames(const std::function<void(Frame *)> &workFunction);

    void recordFrame(unsigned int width, unsigned int height);

    void reset();

    bool hasFrames() { return head->buffer != nullptr; }

    unsigned int getWidth() const { return head->width; }

    unsigned int getHeight() const { return head->height; }

  private:
    Frame *head = nullptr;
    Frame *tail = nullptr;
};
