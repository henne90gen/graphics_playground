#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <imgui.h>

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

    void setDimensions(const unsigned int width, const unsigned int height) {
        this->width = width;
        this->height = height;
        setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    }

protected:
    GLFWwindow *window;

    inline float getAspectRatio() { return aspectRatio; }

    inline unsigned int getWidth() { return width; }

    inline unsigned int getHeight() { return height; }

private:
    const char *name;
    unsigned int width = 0;
    unsigned int height = 0;
    float aspectRatio = 16.0f / 9.0f;
    std::function<void(void)> &backToMainMenu;

    void setAspectRatio(const float ratio) { this->aspectRatio = ratio; }
};
