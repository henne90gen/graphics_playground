#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <imgui.h>
#include "util/InputData.h"

class Scene {
public:
    Scene(GLFWwindow *window, std::function<void(void)> &backToMainMenu, const char *name)
            : window(window), name(name), backToMainMenu(backToMainMenu) {};

    virtual ~Scene() = default;;

    void renderBackMenu();

    virtual void setup() = 0;

    virtual void tick() = 0;

    virtual void destroy() = 0;

    const char *getName() { return name; }

    void setDimensions(const unsigned int _width, const unsigned int _height) {
        this->width = _width;
        this->height = _height;
        this->aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        onAspectRatioChange();
    }

    void setInputData(InputData *_input) {
        this->input = _input;
    }

protected:
    GLFWwindow *window;

    inline float getAspectRatio() { return aspectRatio; }

    virtual void onAspectRatioChange() {};

    inline unsigned int getWidth() { return width; }

    inline unsigned int getHeight() { return height; }

    inline InputData *getInput() { return input; }

private:
    const char *name;
    unsigned int width = 0;
    unsigned int height = 0;
    InputData *input;
    float aspectRatio = 16.0f / 9.0f;
    std::function<void(void)> &backToMainMenu;
};
