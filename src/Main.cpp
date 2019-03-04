#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <iostream>
#include <vector>

#include "ImGui.h"
#include "MainMenu.h"
#include "Scene.h"
#include "TestScene.h"

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    std::cout << "Mouse " << button << std::endl;
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    std::cout << "Scrolled " << xoffset << ", " << yoffset << std::endl;
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    std::cout << "Key " << key << std::endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
        glfwSetWindowShouldClose(window, 1);
    }
}

void charCallback(GLFWwindow *window, unsigned int c) { std::cout << "Entered character " << c << std::endl; }

void installCallbacks(GLFWwindow *window) {
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCharCallback(window, charCallback);
}

void updateViewport(GLFWwindow *window) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0, 0, 0, 1);
}

int main() {
    GLFWwindow *window;

    if (!glfwInit()) {
        return -1;
    }

    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    installCallbacks(window);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    initImGui(window);

    unsigned int currentSceneIndex = 0;

    std::vector<Scene *> scenes = std::vector<Scene *>();
    MainMenu mainMenu = MainMenu(scenes, &currentSceneIndex);

    std::function<void(void)> backToMainMenu = [&currentSceneIndex, &mainMenu]() { 
        currentSceneIndex = 0;
        mainMenu.activate();
    };
    scenes.push_back(new TestScene(window, backToMainMenu));

    while (!glfwWindowShouldClose(window)) {
        startImGuiFrame();
        updateViewport(window);

        if (mainMenu.isActive()) {
            mainMenu.tick();
        } else {
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
