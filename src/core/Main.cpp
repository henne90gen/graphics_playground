#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <iostream>
#include <omp.h>
#include <thread>

#include "Scene.h"
#include "util/ImGuiUtils.h"
#include "util/InputData.h"

#ifdef WITH_SCREEN_RECORDING
#include <ScreenRecorder.h>
#endif

const unsigned int INITIAL_WINDOW_WIDTH = 1200;
const unsigned int INITIAL_WINDOW_HEIGHT = 900;

void setOmpThreadLimit() {
    auto coreCount = std::thread::hardware_concurrency();
    if (coreCount == 0) {
        coreCount = 2;
    }
    // setting to one less than the core count, to leave one core empty for the rendering thread
    omp_set_num_threads(coreCount - 1);
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
    auto *scene = reinterpret_cast<Scene *>(glfwGetWindowUserPointer(window));
    scene->onMouseButton(button, action, mods);
}

void cursorPosCallback(GLFWwindow *window, double xPos, double yPos) {
    auto *scene = reinterpret_cast<Scene *>(glfwGetWindowUserPointer(window));
    scene->onCursorPos(xPos, yPos);
}

void scrollCallback(GLFWwindow *window, double xOffset, double yOffset) {
    auto *scene = reinterpret_cast<Scene *>(glfwGetWindowUserPointer(window));
    scene->onScroll(xOffset, yOffset);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto *scene = reinterpret_cast<Scene *>(glfwGetWindowUserPointer(window));
    scene->onKey(key, scancode, action, mods);
}

void charCallback(GLFWwindow *window, unsigned int c) {
    auto *scene = reinterpret_cast<Scene *>(glfwGetWindowUserPointer(window));
    scene->onCharacterTyped(c);
}

void resizeCallback(GLFWwindow *window, int width, int height) {
    auto *scene = reinterpret_cast<Scene *>(glfwGetWindowUserPointer(window));
    scene->onWindowResize(width, height);
}

void installCallbacks(GLFWwindow *window) {
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCharCallback(window, charCallback);
    glfwSetWindowSizeCallback(window, resizeCallback);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
}

#ifdef WITH_SCREEN_RECORDER
void renderCaptureMenu(ScreenRecorder *recorder) {
    ImGui::Begin("Recording Menu");
    ImGui::SetWindowPos(ImVec2(0, 0));
    const unsigned int windowWidth = 150;
    const unsigned int windowHeight = 100;
    ImGui::SetWindowSize(ImVec2(windowWidth, windowHeight));

    if (ImGui::Button("Take Screenshot")) {
        recorder->takeScreenshot();
    }

    ImGui::RadioButton("GIF", reinterpret_cast<int *>(&recorder->recordingType), ScreenRecorder::RecordingType::GIF);
    ImGui::SameLine();
    ImGui::RadioButton("MP4", reinterpret_cast<int *>(&recorder->recordingType), ScreenRecorder::RecordingType::MP4);

    if (recorder->isRecording()) {
        if (ImGui::Button("Stop Recording")) {
            recorder->stopRecording();
        }
    } else {
        if (ImGui::Button("Start Recording")) {
            recorder->startRecording();
        }
    }

    ImGui::End();
}
#endif

int runScene(Scene *scene) {
    setOmpThreadLimit();

    GLFWwindow *window = nullptr;

    if (glfwInit() == 0) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, scene->getName().c_str(), nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return 1;
    }

    // to disable vsync uncomment this line
    //    glfwSwapInterval(0);

#ifdef WITH_SCREEN_RECORDER
    ScreenRecorder recorder = {};
    glfwSetWindowUserPointer(window, &recorder);
#endif

    glfwSetWindowUserPointer(window, scene);
    // triggering it once "manually" to ensure the aspect ratio is set up correctly
    scene->onWindowResize(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
    installCallbacks(window);

    // ImGui installs its own glfw callbacks, which will then call our previously installed callbacks
    initImGui(window);

    scene->setup(window);

    glEnable(GL_DEPTH_TEST);

    enableOpenGLDebugging();

    while (glfwWindowShouldClose(window) == 0) {
        glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // NOLINT(hicpp-signed-bitwise)

        startImGuiFrame();

        scene->internalTick();

#ifdef WITH_SCREEN_RECORDER
        renderCaptureMenu(&recorder);
        recorder.tick(windowWidth, windowHeight);
#endif

        finishImGuiFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
