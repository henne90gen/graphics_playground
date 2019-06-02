#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "scenes/LegacyTriangle.h"
#include "scenes/TestScene.h"
#include "scenes/cube/Cube.h"
#include "scenes/gamma_calculation/GammaCalculation.h"
#include "scenes/landscape/Landscape.h"
#include "scenes/rubiks_cube/RubiksCubeScene.h"
#include "scenes/texture_demo/TextureDemo.h"
#include "scenes/triangle/Triangle.h"
#include "scenes/marching_cubes/MarchingCubesScene.h"
#include "scenes/font_demo/FontDemo.h"
#include "util/ImGuiUtils.h"
#include "util/MainMenu.h"

const unsigned int INITIAL_WINDOW_WIDTH = 1200;
const unsigned int INITIAL_WINDOW_HEIGHT = 900;

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

//    mainMenu.goToScene(static_cast<unsigned int>(scenes.size()) - 1);
    mainMenu.goToScene(8);

    GL_Call(glEnable(GL_DEPTH_TEST));

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
