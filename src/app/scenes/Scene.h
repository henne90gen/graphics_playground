#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <imgui.h>
#include "util/InputData.h"

struct SceneData {
    GLFWwindow *window;

    InputData *input;

    std::function<void(void)> &backToMainMenu;
    std::function<void(void)> &takeScreenshot;
};

class Scene {
public:
    Scene(SceneData data, const std::string name)
            : name(name), data(data) {};

    virtual ~Scene() = default;;

    void renderBackMenu();

    virtual void setup() = 0;

    virtual void tick() = 0;

    virtual void destroy() = 0;

    const std::string &getName() { return name; }

    void setDimensions(const unsigned int _width, const unsigned int _height) {
        this->width = _width;
        this->height = _height;
        this->aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        onAspectRatioChange();
    }

protected:
    inline float getAspectRatio() { return aspectRatio; }

    virtual void onAspectRatioChange() {};

    inline unsigned int getWidth() { return width; }

    inline unsigned int getHeight() { return height; }

    inline InputData *getInput() { return data.input; }

private:
    const std::string name;
    SceneData data;

    unsigned int width = 0;
    unsigned int height = 0;
    float aspectRatio = 16.0f / 9.0f;
};
