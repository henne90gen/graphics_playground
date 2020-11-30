#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <iostream>
#include <omp.h>
#include <thread>
#include <vector>

#include "Scene.h"
#include "util/ImGuiUtils.h"
#include "util/InputData.h"
#include "util/ScreenRecorder.h"

const unsigned int INITIAL_WINDOW_WIDTH = 1200;
const unsigned int INITIAL_WINDOW_HEIGHT = 900;

InputData input = {};

void setOmpThreadLimit() {
    auto coreCount = std::thread::hardware_concurrency();
    if (coreCount == 0) {
        coreCount = 2;
    }
    // setting to one less than the core count, to leave one core empty for the graphics thread
    omp_set_num_threads(coreCount - 1);
}

void GLAPIENTRY messageCallback(GLenum /*source*/, GLenum type, GLuint /*id*/, GLenum severity, GLsizei /*length*/,
                                const GLchar *message, const void * /*userParam*/) {
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

void enableOpenGLDebugging() {
    // TODO(henne): this does not work on MacOS
    //    GL_Call(glEnable(GL_DEBUG_OUTPUT));
    //    GL_Call(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
    //    GL_Call(glDebugMessageCallback(messageCallback, nullptr));
}

void mouseButtonCallback(GLFWwindow * /*window*/, int button, int action, int /*mods*/) {
    // std::cout << "Mouse " << button << std::endl;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        input.mouse.left = action == GLFW_PRESS;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        input.mouse.right = action == GLFW_PRESS;
    }
}

void cursorPosCallback(GLFWwindow * /*window*/, double xpos, double ypos) {
    input.mouse.pos.x = xpos;
    input.mouse.pos.y = ypos;
}

void scrollCallback(GLFWwindow * /*window*/, double /*xoffset*/, double /*yoffset*/) {
    // std::cout << "Scrolled " << xoffset << ", " << yoffset << std::endl;
}

void keyCallback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/) {
    // std::cout << "Key " << key << std::endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
        glfwSetWindowShouldClose(window, 1);
    } else if (key == GLFW_KEY_F11 && action == GLFW_RELEASE) {
        auto *sceneData = static_cast<SceneData *>(glfwGetWindowUserPointer(window));
        sceneData->recorder->takeScreenshot();
    } else {
        bool isDown = (action == GLFW_PRESS || action == GLFW_REPEAT) && action != GLFW_RELEASE;
        input.keyboard.keys[key] = isDown;
    }
}

void charCallback(GLFWwindow * /*window*/, unsigned int /*c*/) {
    //  std::cout << "Entered character " << c << std::endl;
}

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

int runScene(Scene *scene) {
    setOmpThreadLimit();

    GLFWwindow *window = nullptr;

    if (glfwInit() == 0) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, "Graphics Playground", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    installCallbacks(window);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return 1;
    }

    initImGui(window);

    ScreenRecorder recorder = {};
    SceneData sceneData = {window, &input, &recorder};
    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
scene->setup(windowWidth, windowHeight, sceneData);
    glfwSetWindowUserPointer(window, &sceneData);

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

        recorder.tick(windowWidth, windowHeight);

        finishImGuiFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
