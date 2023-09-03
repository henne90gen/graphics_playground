#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>

#if OpenMP_ENABLED
#include <omp.h>
#endif

#if EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "Scene.h"
#include "util/ImGuiUtils.h"
#include "util/InputData.h"
#include "util/OpenGLUtils.h"

#ifdef WITH_SCREEN_RECORDING
#include <ScreenRecorder.h>
#endif

const unsigned int INITIAL_WINDOW_WIDTH = 1200;
const unsigned int INITIAL_WINDOW_HEIGHT = 900;
GLFWwindow *glfwWindow = nullptr;

void setOmpThreadLimit() {
#if OpenMP_ENABLED
    auto coreCount = std::thread::hardware_concurrency();
    if (coreCount == 0) {
        coreCount = 2;
    }
    // setting to one less than the core count, to leave one core empty for the rendering thread
    omp_set_num_threads(coreCount - 1);
#endif
}

void GLAPIENTRY messageCallback(GLenum /*source*/, GLenum type, GLuint /*id*/, GLenum severity, GLsizei /*length*/,
                                const GLchar *message, const void * /*userParam*/) {
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

void enableOpenGLDebugging() {
#if 0
    // TODO(henne): this does not work on MacOS
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(messageCallback, nullptr);
#endif
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    auto *scene = reinterpret_cast<Scene *>(glfwGetWindowUserPointer(window));
    scene->onMouseButton(button, action, mods);
}

void cursorPosCallback(GLFWwindow *window, double xPos, double yPos) {
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    auto *scene = reinterpret_cast<Scene *>(glfwGetWindowUserPointer(window));
    scene->onCursorPos(xPos, yPos);
}

void scrollCallback(GLFWwindow *window, double xOffset, double yOffset) {
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    auto *scene = reinterpret_cast<Scene *>(glfwGetWindowUserPointer(window));
    scene->onScroll(xOffset, yOffset);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
        glfwSetWindowShouldClose(window, 1);
    }

    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        return;
    }

    auto *scene = reinterpret_cast<Scene *>(glfwGetWindowUserPointer(window));
    scene->onKey(key, scancode, action, mods);
}

void charCallback(GLFWwindow *window, unsigned int c) {
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        return;
    }

    auto *scene = reinterpret_cast<Scene *>(glfwGetWindowUserPointer(window));
    scene->onCharacterTyped(c);
}

void resizeCallback(GLFWwindow *window, int width, int height) {
    auto *scene = reinterpret_cast<Scene *>(glfwGetWindowUserPointer(window));
    scene->onWindowResize(width, height);
}

#if EMSCRIPTEN
extern "C" {
EMSCRIPTEN_KEEPALIVE void emscriptenCanvasResized(int width, int height) {
    glfwSetWindowSize(glfwWindow, width, height);
}
};
#endif

void installCallbacks(GLFWwindow *window) {
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCharCallback(window, charCallback);
    glfwSetWindowSizeCallback(window, resizeCallback);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
}

void runMainLoop(void *data) {
    auto *scene = static_cast<Scene *>(data);

    glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // NOLINT(hicpp-signed-bitwise)

    startImGuiFrame();

    scene->internalTick();

    finishImGuiFrame();

    glfwSwapBuffers(glfwWindow);
    glfwPollEvents();
}

int runScene(Scene *scene) {
    setOmpThreadLimit();

    if (glfwInit() == 0) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindow =
          glfwCreateWindow(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, scene->getName().c_str(), nullptr, nullptr);
    if (glfwWindow == nullptr) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(glfwWindow);
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return 1;
    }

    GL_Call(const auto openGLVersion = glGetString(GL_VERSION));
    std::cout << "OpenGL Version: " << openGLVersion << std::endl;

    // to disable vsync uncomment this line
    //    glfwSwapInterval(0);

    glfwSetWindowUserPointer(glfwWindow, scene);
    // triggering it once "manually" to ensure the aspect ratio is set up correctly
    scene->onWindowResize(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
    installCallbacks(glfwWindow);

    // ImGui installs its own glfw callbacks, which will then call our previously installed callbacks
    initImGui(glfwWindow);

    scene->setup(glfwWindow);

    glEnable(GL_DEPTH_TEST);

    enableOpenGLDebugging();

#if EMSCRIPTEN
    emscripten_set_main_loop_arg(runMainLoop, scene, 0, 1);
#else
    while (glfwWindowShouldClose(glfwWindow) == 0) {
        runMainLoop(scene);
    }
#endif

    glfwTerminate();
    return 0;
}
