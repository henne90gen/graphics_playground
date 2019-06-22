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
        unsigned int vertexIndex;
        unsigned int textureCoordinateIndex;
        unsigned int normalIndex;
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

    unsigned int fromFile(const std::string &fileName, Model &model) {
        if (fileName.find(".obj") == std::string::npos) {
            std::cerr << fileName << " is not an obj file" << std::endl;
            return 1;
        }

        std::vector<std::string> lines;
        std::ifstream modelStream(fileName, std::ios::in);

        if (!modelStream.is_open()) {
            std::cerr << "Could not open " << fileName << std::endl;
            return 1;
        }

        std::string str;
        while (std::getline(modelStream, str)) {
            lines.push_back(str);
        }

        return fromFileContent(fileName, lines, model);
    }

    void parseTextureCoordinates(const std::string &fileName, unsigned long lineNumber, const std::string &line,
                                 std::vector<glm::vec2> &textureCoordinates);

    void parseMaterialLib(const std::string &fileName, unsigned long lineNumber, const std::string &line,
                          std::map<std::string, std::shared_ptr<Material>> &materials);

    Mesh createIndicesFromFaces(Mesh &mesh, const std::vector<Face> &faces);

    unsigned int fromFileContent(const std::string &fileName, const std::vector<std::string> &lines, Model &model) {
        if (lines.empty()) {
            return 1;
        }

        model.meshes = {};
        model.materials = {};

        Mesh globalMesh = {};
        std::vector<Face> faces = {};

        for (unsigned long lineNumber = 1; lineNumber <= lines.size(); lineNumber++) {
            std::string l = trim_copy(lines[lineNumber - 1]);
            if (l.empty()) {
                continue;
            }

            if (l[0] == '#') {
                continue;
            } else if (l[0] == 'm' && l[1] == 't' && l[2] == 'l') {
                parseMaterialLib(fileName, lineNumber, l, model.materials);
            } else if (l[0] == 'o') {
                if (!globalMesh.name.empty()) {
                    auto mesh = createIndicesFromFaces(globalMesh, faces);
                    model.meshes.push_back(mesh);
                    faces.clear();
                }
                globalMesh.name = l.substr(2);
            } else if (l[0] == 'v' && l[1] == 'n') {
                parseNormal(fileName, lineNumber, l, globalMesh.normals);
            } else if (l[0] == 'v' && l[1] == 't') {
                parseTextureCoordinates(fileName, lineNumber, l, globalMesh.textureCoordinates);
            } else if (l[0] == 'v') {
                parseVertex(fileName, lineNumber, l, globalMesh.vertices);
            } else if (l[0] == 'f') {
                parseFace(fileName, lineNumber, l, faces);
            } else if (l[0] == 'u' && l[1] == 's' && l[2] == 'e') {
                globalMesh.material = model.materials[l.substr(7)];
            } else if (l[0] == 's') {
                // ignore this for now
            } else {
                std::cout << "Could not parse line in " << fileName << ": " << l << std::endl;
            }
        }

        auto mesh = createIndicesFromFaces(globalMesh, faces);
        model.meshes.push_back(mesh);
        return 0;
    }

    Mesh createIndicesFromFaces(Mesh &mesh, const std::vector<Face> &faces) {
        std::vector<glm::vec3> vertices = {};
        std::vector<glm::vec3> normals = {};
        std::vector<glm::vec2> textureCoordinates = {};
        std::vector<glm::ivec3> indices = {};

        unsigned int index = 0;
        for (auto &face : faces) {
            glm::ivec3 triangle = {};
            unsigned int faceIndex = 0;
            for (auto &vertex : face.vertices) {
                vertices.push_back(mesh.vertices[vertex.vertexIndex - 1]);
                if (vertex.textureCoordinateIndex) {
                    textureCoordinates.push_back(mesh.textureCoordinates[vertex.textureCoordinateIndex - 1]);
                }
                if (vertex.normalIndex) {
                    normals.push_back(mesh.normals[vertex.normalIndex - 1]);
                }

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

        return {mesh.name, vertices, normals, textureCoordinates, indices, mesh.material};
    }

    void parseMaterialLib(const std::string &fileName, unsigned long lineNumber, const std::string &line,
                          std::map<std::string, std::shared_ptr<Material>> &materials) {
        // TODO use fileName and lineNumber to tell the user which file the material lib is from
        int lastSlash = fileName.find_last_of('/');

        std::string path = fileName.substr(0, lastSlash + 1);
        std::string materialFileName = path + line.substr(7);
        if (materialFileName.find(".mtl") == std::string::npos) {
            std::cerr << materialFileName << " from line " << lineNumber << " is not an mtl file" << std::endl;
            return;
        }

        std::vector<std::string> lines;
        std::ifstream modelStream(materialFileName, std::ios::in);

        if (!modelStream.is_open()) {
            std::cerr << "Could not open " << materialFileName << std::endl;
            return;
        }

        std::string str;
        while (std::getline(modelStream, str)) {
            lines.push_back(str);
        }

        Material currentMaterial = {};
        for (unsigned long materialLineNumber = 1; materialLineNumber <= lines.size(); materialLineNumber++) {
            std::string l = trim_copy(lines[materialLineNumber - 1]);
            if (l.empty()) {
                continue;
            }

            if (l[0] == '#') {
                continue;
            } else if (l[0] == 'n' && l[1] == 'e' && l[2] == 'w') {
                if (!currentMaterial.name.empty()) {
                    materials[currentMaterial.name] = std::make_shared<Material>(currentMaterial);
                }
                currentMaterial = {l.substr(7)};
            } else if (l[0] == 'm' && l[1] == 'a' && l[2] == 'p' && l[4] == 'K' && l[5] == 'd') {
                currentMaterial.diffuseTextureMap = path + l.substr(7);
            }
            // Ns 96.078431
            // Ka 1.000000 1.000000 1.000000
            // Kd 0.640000 0.640000 0.640000
            // Ks 0.500000 0.500000 0.500000
            // Ke 0.000000 0.000000 0.000000
            // Ni 1.000000
            // d 1.000000
            // illum 2
            // map_Kd TestTex.png
        }
        materials[currentMaterial.name] = std::make_shared<Material>(currentMaterial);
    }

    void parseTextureCoordinates(const std::string &fileName, unsigned long lineNumber, const std::string &line,
                                 std::vector<glm::vec2> &textureCoordinates) {
        std::string l = line.substr(3);

        std::istringstream iss(l);
        std::vector<std::string> tokens = {
                std::istream_iterator<std::string>{iss},
                std::istream_iterator<std::string>{}
        };

        if (tokens.size() != 2) {
            std::cout << "Malformed texture coordinate definition in " << fileName << " on line " << lineNumber
                      << std::endl;
            return;
        }

        glm::vec2 textureCoordinate;
        textureCoordinate.x = std::strtof(tokens[0].c_str(), nullptr);
        textureCoordinate.y = std::strtof(tokens[1].c_str(), nullptr);
        textureCoordinates.push_back(textureCoordinate);
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
            return;
        }

        std::vector<FaceVertex> faceVertices = {};
        std::regex slashRegex("/");
        for (auto &s : tokens) {
            std::vector<std::string> parts = {
                    std::sregex_token_iterator(s.begin(), s.end(), slashRegex, -1), {}
            };
            if (parts.size() != 3) {
                std::cout << "Could not parse face vertex in " << fileName << " on line " << lineNumber << std::endl;
                return;
            }
            FaceVertex faceVertex = {};
            faceVertex.vertexIndex = std::strtoul(parts[0].c_str(), nullptr, 10);
            faceVertex.textureCoordinateIndex = std::strtoul(parts[1].c_str(), nullptr, 10);
            faceVertex.normalIndex = std::strtoul(parts[2].c_str(), nullptr, 10);
            faceVertices.push_back(faceVertex);
        }
        faces.push_back({faceVertices});
    }
}
