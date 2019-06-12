#include "ModelLoader.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>
#include <regex>

#include "util/StringUtils.h"

namespace ModelLoader {

    struct FaceVertex {
        unsigned int vertex;
        unsigned int textureCoordinate;
        unsigned int normal;
    };

    struct Face {
        std::vector<FaceVertex> vertices;
    };

    void parseVertex(const std::string &fileName, unsigned long lineNumber, const std::string &line,
                     std::vector<glm::vec3> &vertices);

    void parseNormal(const std::string &fileName, unsigned long lineNumber, const std::string &line,
                     std::vector<glm::vec3> &normals);

    void parseFace(const std::string &fileName, unsigned long lineNumber, const std::string &line,
                   std::vector<Face> &faces);

    Model fromFile(const std::string &fileName) {
        std::vector<std::string> lines;
        std::ifstream modelStream(fileName, std::ios::in);

        if (!modelStream.is_open()) {
            std::cerr << "Could not open " << fileName << std::endl;
        }

        std::string str;
        while (std::getline(modelStream, str)) {
            lines.push_back(str);
        }

        return fromFileContent(fileName, lines);
    }

    Model fromFileContent(const std::string &fileName, const std::vector<std::string> &lines) {
        if (lines.empty()) {
            return {};
        }
        std::vector<glm::vec3> parsedVertices = {};
        std::vector<glm::vec3> parsedNormals = {};
        std::vector<glm::vec2> parsedTextureCoordinates = {};
        std::vector<Face> faces = {};

        for (unsigned long lineNumber = 1; lineNumber <= lines.size(); lineNumber++) {
            std::string l = trim_copy(lines[lineNumber - 1]);
            if (l.empty()) {
                continue;
            }

            if (l[0] == '#') {
                continue;
            } else if (l[0] == 'v' && l[1] != 'n') {
                parseVertex(fileName, lineNumber, l, parsedVertices);
            } else if (l[0] == 'v' && l[1] == 'n') {
                parseNormal(fileName, lineNumber, l, parsedNormals);
            } else if (l[0] == 'f') {
                parseFace(fileName, lineNumber, l, faces);
            } else if (l[0] == 'o' || l[0] == 's') {
                // ignore these for now
            } else {
                std::cout << "Could not parse line in " << fileName << ": " << l << std::endl;
            }
        }

        std::vector<glm::ivec3> indices = {};
        std::vector<glm::vec3> vertices = {};
        std::vector<glm::vec3> normals = {};
        std::vector<glm::vec2> textureCoordinates = {};

        std::map<std::pair<unsigned int, unsigned int>, unsigned int> vertexNormalMap;
        unsigned int index = 0;
        for (auto &face : faces) {
            glm::ivec3 triangle = {};
            glm::ivec3 triangle2 = {};
            unsigned int faceIndex = 0;
            for (auto &vertex : face.vertices) {
                const auto &existingIndexItr = vertexNormalMap.find(std::make_pair(vertex.vertex, vertex.normal));
                if (existingIndexItr != vertexNormalMap.end()) {
                    unsigned int existingIndex = existingIndexItr->second;
                    if (faceIndex == 0) {
                        triangle.x = existingIndex;
                    } else if (faceIndex == 1) {
                        triangle.y = existingIndex;
                    } else if (faceIndex == 2) {
                        triangle.z = existingIndex;
                    }
                    faceIndex++;
                    continue;
                }

                vertices.push_back(parsedVertices[vertex.vertex - 1]);
                normals.push_back(parsedNormals[vertex.normal - 1]);
                vertexNormalMap[std::make_pair(vertex.vertex, vertex.normal)] = index;
                if (faceIndex == 0) {
                    triangle.x = index;
                } else if (faceIndex == 1) {
                    triangle.y = index;
                } else if (faceIndex == 2) {
                    triangle.z = index;
                }
                index++;
                faceIndex++;
            }
            indices.push_back(triangle);
        }

        return {vertices, normals, textureCoordinates, indices};
    }

    void parseVertex(const std::string &fileName, unsigned long lineNumber, const std::string &line,
                     std::vector<glm::vec3> &vertices) {
        std::string l = line.substr(2);

        std::istringstream iss(l);
        std::vector<std::string> tokens = {
                std::istream_iterator<std::string>{iss},
                std::istream_iterator<std::string>{}
        };

        if (tokens.size() != 3) {
            std::cout << "Malformed vertex definition in " << fileName << " on line " << lineNumber << std::endl;
            return;
        }

        glm::vec3 vertex;
        vertex.x = std::strtof(tokens[0].c_str(), nullptr);
        vertex.y = std::strtof(tokens[1].c_str(), nullptr);
        vertex.z = std::strtof(tokens[2].c_str(), nullptr);
        vertices.push_back(vertex);
    }

    void parseNormal(const std::string &fileName, unsigned long lineNumber, const std::string &line,
                     std::vector<glm::vec3> &normals) {
        std::string l = line.substr(3);

        std::istringstream iss(l);
        std::vector<std::string> tokens = {
                std::istream_iterator<std::string>{iss},
                std::istream_iterator<std::string>{}
        };

        if (tokens.size() != 3) {
            std::cout << "Malformed normal definition in " << fileName << " on line " << lineNumber << std::endl;
            return;
        }

        glm::vec3 normal;
        normal.x = std::strtof(tokens[0].c_str(), nullptr);
        normal.y = std::strtof(tokens[1].c_str(), nullptr);
        normal.z = std::strtof(tokens[2].c_str(), nullptr);
        normals.push_back(normal);
    }

    void parseFace(const std::string &fileName, unsigned long lineNumber, const std::string &line,
                   std::vector<Face> &faces) {
        std::string l = line.substr(2);

        std::istringstream iss(l);
        std::vector<std::string> tokens = {
                std::istream_iterator<std::string>{iss},
                std::istream_iterator<std::string>{}
        };
        if (tokens.size() != 3) {
            std::cout << "Malformed face definition in " << fileName << " on line " << lineNumber << std::endl;
        }

        std::vector<FaceVertex> faceVertices = {};
        std::regex slashRegex("/");
        for (auto &s : tokens) {
            std::vector<std::string> parts = {
                    std::sregex_token_iterator(s.begin(), s.end(), slashRegex, -1), {}
            };
            if (parts.size() != 3) {
                std::cout << "Could not parse face vertex in " << fileName << " on line " << lineNumber << std::endl;
            }
            FaceVertex faceVertex = {};
            faceVertex.vertex = std::strtoul(parts[0].c_str(), nullptr, 10);
            faceVertex.textureCoordinate = std::strtoul(parts[1].c_str(), nullptr, 10);
            faceVertex.normal = std::strtoul(parts[2].c_str(), nullptr, 10);
            faceVertices.push_back(faceVertex);
        }
        faces.push_back({faceVertices});
    }
}