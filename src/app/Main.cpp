#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <util/ScreenRecorder.h>
#include <vector>

#include "scenes/a_star/AStar.h"
#include "scenes/bloom_effect/BloomEffect.h"
#include "scenes/cube/Cube.h"
#include "scenes/font_demo/FontDemo.h"
#include "scenes/fourier_transform/FourierTransform.h"
#include "scenes/framebuffer_demo/FramebufferDemo.h"
#include "scenes/gamma_calculation/GammaCalculation.h"
#include "scenes/landscape/Landscape.h"
#include "scenes/legacy_triangle/LegacyTriangle.h"
#include "scenes/light_demo/LightDemo.h"
#include "scenes/marching_cubes/MarchingCubesScene.h"
#include "scenes/meta_balls/MetaBallsScene.h"
#include "scenes/model_loading/ModelLoading.h"
#include "scenes/normal_mapping/NormalMapping.h"
#include "scenes/ray_tracing/RayTracing.h"
#include "scenes/rubiks_cube/RubiksCubeScene.h"
#include "scenes/shadows_2d/Shadows2D.h"
#include "scenes/spot_light/SpotLight.h"
#include "scenes/test_scene/TestScene.h"
#include "scenes/texture_demo/TextureDemo.h"
#include "scenes/triangle/Triangle.h"
#include "util/ImGuiUtils.h"
#include "util/InputData.h"
#include "util/MainMenu.h"

const unsigned int INITIAL_WINDOW_WIDTH = 1200;
const unsigned int INITIAL_WINDOW_HEIGHT = 900;

InputData input = {};

void enableOpenGLDebugging();

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
        sceneData->recorder.takeScreenshot();
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

int main() {
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

    unsigned int currentSceneIndex = 0;

    std::vector<Scene *> scenes = std::vector<Scene *>();
    MainMenu mainMenu = MainMenu(window, scenes, &currentSceneIndex);

    std::function<void(void)> backToMainMenu = [&currentSceneIndex, &mainMenu, &scenes]() {
        scenes[currentSceneIndex]->destroy();
        currentSceneIndex = 0;
        mainMenu.activate();
    };
    ScreenRecorder recorder = {};
    SceneData sceneData = {window, &input, backToMainMenu, recorder};
    glfwSetWindowUserPointer(window, &sceneData);

    GL_Call(glEnable(GL_DEPTH_TEST));

    scenes.push_back(new TestScene(sceneData));          // 0
    scenes.push_back(new LegacyTriangle(sceneData));     // 1
    scenes.push_back(new Triangle(sceneData));           // 2
    scenes.push_back(new TextureDemo(sceneData));        // 3
    scenes.push_back(new GammaCalculation(sceneData));   // 4
    scenes.push_back(new Cube(sceneData));               // 5
    scenes.push_back(new Landscape(sceneData));          // 6
    scenes.push_back(new RubiksCubeScene(sceneData));    // 7
    scenes.push_back(new MarchingCubesScene(sceneData)); // 8
    scenes.push_back(new FontDemo(sceneData));           // 9
    scenes.push_back(new ModelLoading(sceneData));       // 10
    scenes.push_back(new LightDemo(sceneData));          // 11
    scenes.push_back(new FourierTransform(sceneData));   // 12
    scenes.push_back(new NormalMapping(sceneData));      // 13
    scenes.push_back(new AStar(sceneData));              // 14
    scenes.push_back(new Shadows2D(sceneData));          // 15
    scenes.push_back(new RayTracing(sceneData));         // 16
    scenes.push_back(new FramebufferDemo(sceneData));    // 17
    scenes.push_back(new BloomEffect(sceneData));        // 18
    scenes.push_back(new SpotLight(sceneData));          // 19
    scenes.push_back(new MetaBallsScene(sceneData));     // 20

    // mainMenu.goToScene(static_cast<unsigned int>(scenes.size()) - 1);
    mainMenu.goToScene(16);

    enableOpenGLDebugging();

    while (glfwWindowShouldClose(window) == 0) {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // NOLINT(hicpp-signed-bitwise)

        startImGuiFrame();

        if (mainMenu.isActive()) {
            mainMenu.render();
        } else {
            int windowWidth = 0;
            int windowHeight = 0;
            glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
            scenes[currentSceneIndex]->tick(windowWidth, windowHeight);

            recorder.tick(windowWidth, windowHeight);
        }

        finishImGuiFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void GLAPIENTRY MessageCallback(GLenum /*source*/, GLenum type, GLuint /*id*/, GLenum severity, GLsizei /*length*/,
                                const GLchar *message, const void * /*userParam*/) {
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

void enableOpenGLDebugging() {
    // TODO(henne): this does not work on MacOS
    //    GL_Call(glEnable(GL_DEBUG_OUTPUT));
    //    GL_Call(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
    //    GL_Call(glDebugMessageCallback(MessageCallback, nullptr));
}
