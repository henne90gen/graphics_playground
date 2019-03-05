#include "scenes/triangle/Triangle.h"

#include <glad/glad.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "ImGuiUtils.h"
#include "OpenGLUtils.h"

GLuint loadShader(GLuint shaderType, const char *file_path) {

    std::ifstream infile(file_path);
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(infile, line)) {
        lines.push_back(line);
    }

    int *lineLengths = (int *)malloc(lines.size() * sizeof(int));
    for (unsigned int i = 0; i < lines.size(); i++) {
        lineLengths[i] = lines[i].size() + 1;
    }

    char **linesArray = (char **)malloc(lines.size() * sizeof(char *));
    for (unsigned int i = 0; i < lines.size(); i++) {
        linesArray[i] = (char *)malloc((lines[i].size() + 1) * sizeof(char));
        for (unsigned int j = 0; j < lines[i].size(); j++) {
            linesArray[i][j] = lines[i][j];
        }
        linesArray[i][lines[i].size()] = '\n';
    }

    GL_Call(GLuint shaderId = glCreateShader(shaderType));

    unsigned int lineCount = lines.size();
    GL_Call(glShaderSource(shaderId, lineCount, linesArray, lineLengths));
    GL_Call(glCompileShader(shaderId));

    int error;
    int infoLogLength;
    GL_Call(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &error));
    GL_Call(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength));
    if (infoLogLength > 0) {
        std::vector<char> vertexShaderErrorMessage(infoLogLength + 1);
        GL_Call(glGetShaderInfoLog(shaderId, infoLogLength, NULL, &vertexShaderErrorMessage[0]));
        std::cout << &vertexShaderErrorMessage[0] << std::endl;
    }

    for (unsigned int i = 0; i < lines.size(); i++) {
        free(linesArray[i]);
    }
    free(linesArray);
    free(lineLengths);
    return shaderId;
}

GLuint loadShaders(const char *vertex_file_path, const char *fragment_file_path) {
    GL_Call(GLuint programId = glCreateProgram());
    GLuint vertexShaderId = loadShader(GL_VERTEX_SHADER, vertex_file_path);
    GLuint fragmentShaderId = loadShader(GL_FRAGMENT_SHADER, fragment_file_path);
    GL_Call(glAttachShader(programId, vertexShaderId));
    GL_Call(glAttachShader(programId, fragmentShaderId));

    GL_Call(glLinkProgram(programId));

    int error;
    int infoLogLength;
    GL_Call(glGetProgramiv(programId, GL_LINK_STATUS, &error));
    GL_Call(glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength));
    if (infoLogLength > 0) {
        std::vector<char> programErrorMessage(infoLogLength + 1);
        GL_Call(glGetProgramInfoLog(programId, infoLogLength, NULL, &programErrorMessage[0]));
        std::cout << &programErrorMessage[0] << std::endl;
    }

    GL_Call(glDetachShader(programId, vertexShaderId));
    GL_Call(glDetachShader(programId, fragmentShaderId));

    GL_Call(glDeleteShader(vertexShaderId));
    GL_Call(glDeleteShader(fragmentShaderId));

    return programId;
}

void Triangle::tick() {
    static float color[3] = {1.0, 1.0, 1.0};
    static float vertices[6] = {-1, -1, 0, 1, 1, -1};
    pickColorAndVertices(color, vertices);

    GLuint programId =
        loadShaders("../src/scenes/triangle/Triangle.vertex", "../src/scenes/triangle/Triangle.fragment");

    GL_Call(glUseProgram(programId));

    GLuint vertexbuffer;
    GL_Call(glGenBuffers(1, &vertexbuffer));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer));
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    GL_Call(GLuint positionLocation = glGetAttribLocation(programId, "position"));
    GL_Call(glEnableVertexAttribArray(positionLocation));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer));
    GL_Call(glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, (void *)0));

    GLuint colorBuffer;
    GL_Call(glGenBuffers(1, &colorBuffer));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, colorBuffer));
    float colors[9] = {};
    for (int i = 0; i < 9; i++) {
        colors[i] = color[i % 3];
    }
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW));

    GL_Call(GLuint colorLocation = glGetAttribLocation(programId, "color"));
    GL_Call(glEnableVertexAttribArray(colorLocation));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, colorBuffer));
    GL_Call(glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_TRUE, 0, (void *)0));

    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 3));

    GL_Call(glDisableVertexAttribArray(1));
    GL_Call(glDisableVertexAttribArray(0));

    GL_Call(glUseProgram(0));
}
