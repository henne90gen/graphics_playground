#include "scenes/AdvancedTriangle.h"

#include <glad/glad.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "ImGuiUtils.h"

GLuint loadShader(GLuint shaderType, const char *file_path) {
    GLuint shaderId = glCreateShader(shaderType);

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

    unsigned int lineCount = lines.size();
    glShaderSource(shaderId, lineCount, linesArray, lineLengths);
    glCompileShader(shaderId);

    int error;
    int infoLogLength;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &error);
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> vertexShaderErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(shaderId, infoLogLength, NULL, &vertexShaderErrorMessage[0]);
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
    GLuint programId = glCreateProgram();
    GLuint vertexShaderId = loadShader(GL_VERTEX_SHADER, vertex_file_path);
    GLuint fragmentShaderId = loadShader(GL_FRAGMENT_SHADER, fragment_file_path);
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);

    int error;
    int infoLogLength;
    glGetProgramiv(programId, GL_LINK_STATUS, &error);
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> programErrorMessage(infoLogLength + 1);
        glGetProgramInfoLog(programId, infoLogLength, NULL, &programErrorMessage[0]);
        std::cout << &programErrorMessage[0] << std::endl;
    }

    glDetachShader(programId, vertexShaderId);
    glDetachShader(programId, fragmentShaderId);

    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);

    return programId;
}

void AdvancedTriangle::tick() {
    static float color[3] = {1.0, 1.0, 1.0};
    static float vertices[6] = {-1, -1, 0, 1, 1, -1};
    pickColorAndVertices(color, vertices);

    GLuint programId = loadShaders("../src/scenes/AdvancedTriangle.vertex", "../src/scenes/AdvancedTriangle.fragment");

    glUseProgram(programId);

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint positionLocation = glGetAttribLocation(programId, "position");
    glEnableVertexAttribArray(positionLocation);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    float colors[9] = {};
    for (int i = 0; i < 9; i++) {
        colors[i] = color[i % 3];
    }
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    GLuint colorLocation = glGetAttribLocation(programId, "color");
    glEnableVertexAttribArray(colorLocation);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_TRUE, 0, (void *)0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

// GLuint LoadShaders(const char *vertex_file_path, const char *fragment_file_path) {

//     // Create the shaders
//     GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
//     GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

//     // Read the Vertex Shader code from the file
//     std::string VertexShaderCode;
//     std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
//     if (VertexShaderStream.is_open()) {
//         std::stringstream sstr;
//         sstr << VertexShaderStream.rdbuf();
//         VertexShaderCode = sstr.str();
//         VertexShaderStream.close();
//     } else {
//         printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n",
//                vertex_file_path);
//         getchar();
//         return 0;
//     }

//     // Read the Fragment Shader code from the file
//     std::string FragmentShaderCode;
//     std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
//     if (FragmentShaderStream.is_open()) {
//         std::stringstream sstr;
//         sstr << FragmentShaderStream.rdbuf();
//         FragmentShaderCode = sstr.str();
//         FragmentShaderStream.close();
//     }

//     GLint Result = GL_FALSE;
//     int InfoLogLength;

//     // Compile Vertex Shader
//     printf("Compiling shader : %s\n", vertex_file_path);
//     char const *VertexSourcePointer = VertexShaderCode.c_str();
//     glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
//     glCompileShader(VertexShaderID);

//     // Check Vertex Shader
//     glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
//     glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
//     if (InfoLogLength > 0) {
//         std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
//         glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
//         printf("%s\n", &VertexShaderErrorMessage[0]);
//     }

//     // Compile Fragment Shader
//     printf("Compiling shader : %s\n", fragment_file_path);
//     char const *FragmentSourcePointer = FragmentShaderCode.c_str();
//     glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
//     glCompileShader(FragmentShaderID);

//     // Check Fragment Shader
//     glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
//     glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
//     if (InfoLogLength > 0) {
//         std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
//         glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
//         printf("%s\n", &FragmentShaderErrorMessage[0]);
//     }

//     // Link the program
//     printf("Linking program\n");
//     GLuint ProgramID = glCreateProgram();
//     glAttachShader(ProgramID, VertexShaderID);
//     glAttachShader(ProgramID, FragmentShaderID);
//     glLinkProgram(ProgramID);

//     // Check the program
//     glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
//     glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
//     if (InfoLogLength > 0) {
//         std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
//         glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
//         printf("%s\n", &ProgramErrorMessage[0]);
//     }

//     glDetachShader(ProgramID, VertexShaderID);
//     glDetachShader(ProgramID, FragmentShaderID);

//     glDeleteShader(VertexShaderID);
//     glDeleteShader(FragmentShaderID);

//     return ProgramID;
// }
