#pragma once

#include <glm/ext.hpp>
#include <glad/glad.h>

#ifdef WIN32
#define ASSERT(x)             \
        if (!(x))                 \
            __debugbreak()
#else
#define ASSERT(x)             \
        if (!(x))                  \
            __builtin_debugtrap()
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

GLuint loadShader(GLuint shaderType, const char *file_path);

GLuint loadShaders(const char *vertex_file_path, const char *fragment_file_path);

glm::mat4 createViewMatrix(const glm::vec3 &cameraPosition, const glm::vec3 &cameraRotation);
