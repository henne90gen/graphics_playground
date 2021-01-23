#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <iostream>
#include <omp.h>
#include <thread>

#include "Scene.h"
#include "util/ImGuiUtils.h"
#include "util/InputData.h"

#ifdef WITH_SCREEN_RECORDING
#include "util/ScreenRecorder.h"
#endif

const unsigned int INITIAL_WINDOW_WIDTH = 1200;
const unsigned int INITIAL_WINDOW_HEIGHT = 900;

InputData input = {};

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

void mouseButtonCallback(GLFWwindow * /*window*/, int button, int action, int /*mods*/) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        input.mouse.left = action == GLFW_PRESS;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        input.mouse.right = action == GLFW_PRESS;
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        input.mouse.middle = action == GLFW_PRESS;
    }
}

void cursorPosCallback(GLFWwindow * /*window*/, double xpos, double ypos) {
    input.mouse.pos.x = xpos;
    input.mouse.pos.y = ypos;
}

void scrollCallback(GLFWwindow * /*window*/, double /*xoffset*/, double /*yoffset*/) {}

void keyCallback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
        glfwSetWindowShouldClose(window, 1);
    } else if (key == GLFW_KEY_F11 && action == GLFW_RELEASE) {
#ifdef WITH_SCREEN_RECORDER
        auto *recorder = static_cast<ScreenRecorder *>(glfwGetWindowUserPointer(window));
        recorder->takeScreenshot();
#endif
    } else {
        bool isDown = (action == GLFW_PRESS || action == GLFW_REPEAT) && action != GLFW_RELEASE;
        input.keyboard.keys[key] = isDown;
    }
}

void charCallback(GLFWwindow * /*window*/, unsigned int /*c*/) {}

void resizeCallback(GLFWwindow * /*window*/, int width, int height) { glViewport(0, 0, width, height); }

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
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    installCallbacks(window);
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return 1;
    }

    initImGui(window);

    // to disable vsync uncomment this line
    //    glfwSwapInterval(0);

#ifdef WITH_SCREEN_RECORDER
    ScreenRecorder recorder = {};
    glfwSetWindowUserPointer(window, &recorder);
#endif

    SceneData sceneData = {window, &input};
    {
        int windowWidth = 0;
        int windowHeight = 0;
        glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
        scene->setup(windowWidth, windowHeight, sceneData);
    }

    glEnable(GL_DEPTH_TEST);

    enableOpenGLDebugging();

    while (glfwWindowShouldClose(window) == 0) {
        glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // NOLINT(hicpp-signed-bitwise)

        startImGuiFrame();

        int windowWidth = 0;
        int windowHeight = 0;
        glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
        scene->tick(windowWidth, windowHeight);

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
