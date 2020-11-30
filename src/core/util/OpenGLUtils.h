#pragma once

#include <glad/glad.h>
#include <glm/ext.hpp>

#include "InputData.h"
#include "util/AssertUtils.h"

#define GL_UnsafeCall(x, error)                                                                                        \
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

MappedMousePosition mapMouseOntoCanvas(const glm::vec2 &mousePos, const glm::mat4 &transformationMatrix,
                                       unsigned int canvasWidth, unsigned int canvasHeight, unsigned int displayWidth,
                                       unsigned int displayHeight);

void checkFramebufferStatus();