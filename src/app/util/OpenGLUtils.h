#pragma once

#include <glm/ext.hpp>
#include <glad/glad.h>

#include "InputData.h"

#ifdef DEBUG
#ifdef WIN32
#define ASSERT(x)             \
        if (!(x))                 \
            __debugbreak()
#else
#define ASSERT(x)             \
        if (!(x))                  \
            __builtin_trap()
#endif
#else
#define ASSERT(x)
#endif

#define GL_UnsafeCall(x, error)                                                                                                     \
    GL_ClearError();                                                                                                   \
    x;                                                                                                                 \
    error = !GL_LogCall(#x, __FILE__, __LINE__)

#define GL_Call(x)                                                                                                     \
    GL_ClearError();                                                                                                   \
    x;                                                                                                                 \
    ASSERT(GL_LogCall(#x, __FILE__, __LINE__))
#define GL_CheckErrors() ASSERT(GL_LogCall("", __FILE__, __LINE__))

void GL_ClearError();

bool GL_LogCall(const char *function, const char *file, int line);

glm::mat4 createViewMatrix(const glm::vec3 &cameraPosition, const glm::vec3 &cameraRotation);

struct MappedMousePosition {
    glm::vec2 canvasPos;
    glm::vec2 worldPos;
};

MappedMousePosition
mapMouseOntoCanvas(const InputData *input, const glm::mat4 &transformationMatrix, unsigned int canvasWidth,
                   unsigned int canvasHeight, unsigned int displayWidth, unsigned int displayHeight);
