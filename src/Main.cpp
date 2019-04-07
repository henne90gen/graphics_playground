#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "ImGuiUtils.h"
#include "MainMenu.h"
#include "scenes/LegacyTriangle.h"
#include "scenes/TestScene.h"
#include "scenes/cube/Cube.h"
#include "scenes/texture_demo/TextureDemo.h"
#include "scenes/triangle/Triangle.h"
#include "scenes/landscape/Landscape.h"
#include "scenes/rubiks_cube/RubiksCube.h"

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    // std::cout << "Mouse " << button << std::endl;
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    // std::cout << "Scrolled " << xoffset << ", " << yoffset << std::endl;
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // std::cout << "Key " << key << std::endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
        glfwSetWindowShouldClose(window, 1);
    }
}

void charCallback(GLFWwindow *window, unsigned int c) {
    //  std::cout << "Entered character " << c << std::endl;
}

void resizeCallback(GLFWwindow *window, int width, int height) {
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

    if (!glfwInit()) {
        return -1;
    }

    window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    installCallbacks(window);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    initImGui(window);

    unsigned int currentSceneIndex = 0;

    std::vector<Scene *> scenes = std::vector<Scene *>();
    MainMenu mainMenu = MainMenu(window, scenes, &currentSceneIndex);

    std::function<void(void)> backToMainMenu = [&currentSceneIndex, &mainMenu, &scenes]() {
        scenes[currentSceneIndex]->destroy();
        currentSceneIndex = 0;
        mainMenu.activate();
    };
    scenes.push_back(new TestScene(window, backToMainMenu));
    scenes.push_back(new LegacyTriangle(window, backToMainMenu));
    scenes.push_back(new Triangle(window, backToMainMenu));
    scenes.push_back(new TextureDemo(window, backToMainMenu));
    scenes.push_back(new Cube(window, backToMainMenu));
    scenes.push_back(new Landscape(window, backToMainMenu));
    scenes.push_back(new RubiksCube(window, backToMainMenu));

    mainMenu.goToScene((unsigned int) scenes.size() - 1);

    GL_Call(glEnable(GL_DEPTH_TEST));

    while (!glfwWindowShouldClose(window)) {
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0, 0, 0, 1);

        startImGuiFrame();

        if (mainMenu.isActive()) {
            mainMenu.render();
        } else {
            int windowWidth, windowHeight;
            glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
            scenes[currentSceneIndex]->setAspectRatio((float) windowWidth / (float) windowHeight);
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
