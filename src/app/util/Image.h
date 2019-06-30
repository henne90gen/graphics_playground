#pragma once

#include <vector>
#include <string>

class Image {
public:
    Image(std::string fileName) : loaded(false), fileName(std::move(fileName)) {}

    ~Image() = default;

    void load();

    char getBitDepth() const { return bitDepth; }

    char getChannels() const { return channels; }

    unsigned int getWidth() const { return width; }

    unsigned int getHeight() const { return height; }

    const std::vector<unsigned char> getPixels() const { return pixels; }

    bool isLoaded() const { return loaded; }

private:
    unsigned int width{};
    unsigned int height{};
    char channels{};
    char bitDepth{};
    std::vector<unsigned char> pixels;
    bool loaded;
    const std::string fileName;

    int loadPng();

    int loadJpg();
};
