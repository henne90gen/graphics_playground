#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "scenes/cube/Cube.h"
#include "scenes/font_demo/FontDemo.h"
#include "scenes/gamma_calculation/GammaCalculation.h"
#include "scenes/landscape/Landscape.h"
#include "scenes/legacy_triangle/LegacyTriangle.h"
#include "scenes/marching_cubes/MarchingCubesScene.h"
#include "scenes/rubiks_cube/RubiksCubeScene.h"
#include "scenes/test_scene/TestScene.h"
#include "scenes/texture_demo/TextureDemo.h"
#include "scenes/triangle/Triangle.h"
#include "scenes/model_loading/ModelLoading.h"
#include "scenes/light_demo/LightDemo.h"
#include "util/ImGuiUtils.h"
#include "util/MainMenu.h"

const unsigned int INITIAL_WINDOW_WIDTH = 1200;
const unsigned int INITIAL_WINDOW_HEIGHT = 900;

void enableOpenGLDebugging();

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    // std::cout << "Mouse " << button << std::endl;
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    // std::cout << "Scrolled " << xoffset << ", " << yoffset << std::endl;
}

void keyCallback(GLFWwindow *window, int key, int  /*scancode*/, int action, int  /*mods*/) {
    // std::cout << "Key " << key << std::endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
        glfwSetWindowShouldClose(window, 1);
    }
}

void charCallback(GLFWwindow *window, unsigned int c) {
    //  std::cout << "Entered character " << c << std::endl;
}

void resizeCallback(GLFWwindow * /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
}

void installCallbacks(GLFWwindow *window) {
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCharCallback(window, charCallback);
    glfwSetWindowSizeCallback(window, resizeCallback);
}

int main() {
    GLFWwindow *window;

    if (glfwInit() == 0) {
        return -1;
    }

    window = glfwCreateWindow(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, "Hello World", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return -1;
    }

    installCallbacks(window);
    glfwMakeContextCurrent(window);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

    initImGui(window);

    unsigned int currentSceneIndex = 0;

    std::vector<Scene *> scenes = std::vector<Scene *>();
    MainMenu mainMenu = MainMenu(window, scenes, &currentSceneIndex);

    std::function<void(void)> backToMainMenu = [&currentSceneIndex, &mainMenu, &scenes]() {
        scenes[currentSceneIndex]->destroy();
        currentSceneIndex = 0;
        mainMenu.activate();
    };
    scenes.push_back(new TestScene(window, backToMainMenu)); // 0
    scenes.push_back(new LegacyTriangle(window, backToMainMenu)); // 1
    scenes.push_back(new Triangle(window, backToMainMenu)); // 2
    scenes.push_back(new TextureDemo(window, backToMainMenu)); // 3
    scenes.push_back(new GammaCalculation(window, backToMainMenu)); // 4
    scenes.push_back(new Cube(window, backToMainMenu)); // 5
    scenes.push_back(new Landscape(window, backToMainMenu)); // 6
    scenes.push_back(new RubiksCubeScene(window, backToMainMenu)); // 7
    scenes.push_back(new MarchingCubesScene(window, backToMainMenu)); // 8
    scenes.push_back(new FontDemo(window, backToMainMenu)); // 9
    scenes.push_back(new ModelLoading(window, backToMainMenu)); // 10
    scenes.push_back(new LightDemo(window, backToMainMenu)); // 11

    mainMenu.goToScene(static_cast<unsigned int>(scenes.size()) - 1);
//    mainMenu.goToScene(9);

    GL_Call(glEnable(GL_DEPTH_TEST));

    enableOpenGLDebugging();

    while (glfwWindowShouldClose(window) == 0) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // NOLINT(hicpp-signed-bitwise)
        glClearColor(0, 0, 0, 1);

        startImGuiFrame();

        if (mainMenu.isActive()) {
            mainMenu.render();
        } else {
            int windowWidth;
            int windowHeight;
            glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
            scenes[currentSceneIndex]->setDimensions(windowWidth, windowHeight);
            scenes[currentSceneIndex]->renderBackMenu();
            scenes[currentSceneIndex]->tick();
        }

        finishImGuiFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void GLAPIENTRY
MessageCallback(GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar *message,
                const void *userParam) {
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

void enableOpenGLDebugging() {
    GL_Call(glEnable(GL_DEBUG_OUTPUT));
    GL_Call(glDebugMessageCallback(MessageCallback, 0));
}
