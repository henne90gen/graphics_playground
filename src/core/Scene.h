#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <functional>
#include <imgui.h>
#include <utility>

#include "camera/Camera.h"
#include "util/InputData.h"
#include "util/TimeUtils.h"

#ifdef WITH_SCREEN_RECORDING
#include <ScreenRecorder.h>
#endif

class Scene {
  public:
    explicit Scene(std::string name) : name(std::move(name)) {
        auto now = std::chrono::high_resolution_clock::now();
        lastTimeNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch().count();
    };

    virtual ~Scene() = default;

    void setup(GLFWwindow *window);
    void internalTick();

    void renderMetricsMenu();
    void renderCaptureMenu();

    const std::string &getName() { return name; }

    virtual void onWindowResize(int w, int h);
    virtual void onKey(int key, int scancode, int action, int mods);
    virtual void onCursorPos(double xPos, double yPos);
    virtual void onMouseButton(int button, int action, int mods);
    virtual void onCharacterTyped(unsigned int) {}
    virtual void onScroll(double xOffset, double yOffset);

    [[nodiscard]] inline float getAspectRatio() const { return aspectRatio; }
    [[nodiscard]] inline unsigned int getWidth() const { return width; }
    [[nodiscard]] inline unsigned int getHeight() const { return height; }
    [[nodiscard]] inline double getLastFrameTime() const { return timeDelta; }
    [[nodiscard]] inline const InputData &getInput() const { return input; }
    inline Camera &getCamera() { return camera; }
    inline PerformanceCounter *getPerformanceCounter() { return &this->performanceCounter; }

  protected:
    virtual void setup() = 0;
    virtual void tick() = 0;
    virtual void destroy() = 0;
    virtual void onAspectRatioChange() {};

  private:
    const std::string name;
    GLFWwindow *window = nullptr;
    InputData input = {};
    Camera camera = {};
    double timeDelta = 0.0;
    int64_t lastTimeNs = 0L;

    unsigned int width = 0;
    unsigned int height = 0;
    float aspectRatio = 16.0F / 9.0F;

    PerformanceCounter performanceCounter = {};

#ifdef WITH_SCREEN_RECORDING
    ScreenRecorder recorder = {};
#endif
};
