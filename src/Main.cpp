#include "ImGui.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <imgui.h>

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {}

void charCallback(GLFWwindow *window, unsigned int c) {}

void installCallbacks(GLFWwindow *window) {
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCharCallback(window, charCallback);
}

void newFrame(GLFWwindow *window) { ImGui::NewFrame(); }

int main() {
    GLFWwindow *window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Initialize glad */
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    initImGui(window);

    float f = 0.0;
    char buf[255] = {};
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        startImGuiFrame();
        ImGui::ColorEdit3("clear color", (float *)&clear_color);

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        finishImGuiFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
