#include "Model.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>

#include <util/TimeUtils.h>

unsigned int Model::loadFromFile(const std::string &fileName, const std::shared_ptr<Shader> &shader, Model &model) {
    TIME_SCOPE_NAME("loadFromFile");

    model.meshes.clear();

    shader->bind();
    model.rawModel = RawModel();
    unsigned int error = loadRawModelFromFile(fileName, model.rawModel);
    if (error != 0) {
        std::cout << "Could not load model." << std::endl;
        return 1;
    }

    std::unordered_map<std::string, std::shared_ptr<Texture>> textures = {};
    for (auto &rawMesh : model.rawModel.meshes) {
        TextureSettings textureSettings = {};
        textureSettings.dataType = GL_RGBA;
        auto glMesh = OpenGLMesh(textureSettings, shader, true);

        glMesh.vertexArray->bind();

        glMesh.updateMeshVertices(rawMesh, shader);

        glMesh.indexBuffer->bind();
        glMesh.indexBuffer->update(rawMesh.indices);

        glMesh.updateTexture(rawMesh, textures);

        model.meshes.push_back(glMesh);
    }

    model.loaded = true;

    return 0;
}

void OpenGLMesh::updateMeshVertices(const RawMesh &mesh, const std::shared_ptr<Shader> &shader) const {
    //    TIME_SCOPE_NAME("updateMeshVertices");
    bool hasNormals = !mesh.normals.empty();
    shader->setUniform("u_HasNormals", hasNormals);
    bool hasTexture = !mesh.uvs.empty();
    shader->setUniform("u_HasTexture", hasTexture);

    BufferLayout positionLayout = {{Float3, "a_Position"}};
    vertexBuffer->setLayout(positionLayout);
    vertexBuffer->update(mesh.vertices);
    vertexArray->addVertexBuffer(vertexBuffer);

    if (hasNormals) {
        BufferLayout normalLayout = {{Float3, "a_Normal"}};
        normalBuffer->setLayout(normalLayout);
        normalBuffer->update(mesh.normals);
        vertexArray->addVertexBuffer(normalBuffer);
    }

    if (hasTexture) {
        BufferLayout uvLayout = {{Float2, "a_UV"}};
        uvBuffer->setLayout(uvLayout);
        uvBuffer->update(mesh.uvs);
        vertexArray->addVertexBuffer(uvBuffer);
    }
}

void OpenGLMesh::updateTexture(RawMesh &rawMesh, std::unordered_map<std::string, std::shared_ptr<Texture>> &textures) {
    if (rawMesh.material != nullptr) {
        auto itr = textures.find(rawMesh.material->diffuseTextureMap);
        if (itr != textures.end()) {
            texture = itr->second;
            return;
        }
    }

    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    Image image = {};
    if (rawMesh.material == nullptr || !ImageOps::load(rawMesh.material->diffuseTextureMap, image)) {
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        ImageOps::createCheckerBoard(image);
    }
    texture->update(image.pixels.data(), image.width, image.height);
    textures[rawMesh.material->diffuseTextureMap] = texture;
}

/**
 * Trim from start (in place)
 */
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
}

/**
 * Trim from end (in place)
 */
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
}

/**
 * Trim from both ends (in place)
 */
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

/**
 * Trim from both ends (copying)
 */
static inline std::string trim_copy(std::string s) {
    trim(s);
    return s;
}

struct FaceVertex {
    unsigned int vertexIndex = 0;
    unsigned int textureCoordinateIndex = 0;
    unsigned int normalIndex = 0;
};

struct Face {
    std::vector<FaceVertex> vertices;
};

RawMesh createIndicesFromFaces(const RawMesh &mesh, const std::vector<Face> &faces) {
    std::vector<glm::vec3> vertices = {};
    std::vector<glm::vec3> normals = {};
    std::vector<glm::vec2> textureCoordinates = {};
    std::vector<glm::ivec3> indices = {};

    unsigned int index = 0;
    for (const auto &face : faces) {
        std::vector<int> triangleArrangement = {0, 1, 2};
        if (face.vertices.size() == 3) {
            // do nothing
        } else if (face.vertices.size() == 4) {
            triangleArrangement.push_back(0);
            triangleArrangement.push_back(2);
            triangleArrangement.push_back(3);
        } else {
            std::cerr << "Can only parse faces with 3 or 4 vertices" << std::endl;
            return {};
        }

        glm::ivec3 triangle = {};
        for (const auto vertexIndex : triangleArrangement) {
            const auto &vertex = face.vertices[vertexIndex];
            vertices.push_back(mesh.vertices[vertex.vertexIndex - 1]);
            if (vertex.textureCoordinateIndex != 0U) {
                textureCoordinates.push_back(mesh.uvs[vertex.textureCoordinateIndex - 1]);
            }
            if (vertex.normalIndex != 0U) {
                normals.push_back(mesh.normals[vertex.normalIndex - 1]);
            }

            if (vertexIndex == 0) {
                triangle.x = index;
            } else if (vertexIndex == 1) {
                triangle.y = index;
            } else if (vertexIndex == 2) {
                triangle.z = index;
                indices.push_back(triangle);
            }
            index++;
        }
    }

    return {mesh.name, vertices, normals, textureCoordinates, indices, mesh.material};
}

void parseMaterialLib(const std::string &fileName, unsigned long lineNumber, const std::string &line,
                      std::map<std::string, RawMaterial *> &materials) {
    // TODO(henne): use fileName and lineNumber to tell the user which file the material lib is from
    int lastSlash = fileName.find_last_of('/');

    std::string path = fileName.substr(0, lastSlash + 1);
    const int materialLibNameOffset = 7;
    std::string materialFileName = path + line.substr(materialLibNameOffset);
    if (materialFileName.find(".mtl") == std::string::npos) {
        std::cerr << materialFileName << " from line " << lineNumber << " is not an mtl file" << std::endl;
        return;
    }

    std::vector<std::string> lines;
    std::ifstream materialStream(materialFileName, std::ios::in);

    if (!materialStream.is_open()) {
        std::cerr << "Could not open " << materialFileName << std::endl;
        return;
    }

    std::string str;
    while (std::getline(materialStream, str)) {
        lines.push_back(str);
    }

    auto *currentMaterial = new RawMaterial();
    for (unsigned long materialLineNumber = 1; materialLineNumber <= lines.size(); materialLineNumber++) {
        std::string l = trim_copy(lines[materialLineNumber - 1]);
        if (l.empty()) {
            continue;
        }

        if (l[0] == '#') {
            continue;
        }

        if (l[0] == 'n' && l[1] == 'e' && l[2] == 'w') {
            if (!currentMaterial->name.empty()) {
                materials[currentMaterial->name] = currentMaterial;
            }
            const int materialNameOffset = 7;
            currentMaterial = new RawMaterial();
            currentMaterial->name = l.substr(materialNameOffset);
        } else if (l[0] == 'm' && l[1] == 'a' && l[2] == 'p' && l[4] == 'K' && l[5] == 'd') {
            const int diffuseTextureNameOffset = 7;
            currentMaterial->diffuseTextureMap = path + l.substr(diffuseTextureNameOffset);
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
    materials[currentMaterial->name] = currentMaterial;
}

void parseVec2(const std::string &fileName, unsigned long lineNumber, const std::string &line,
               std::vector<glm::vec2> &list) {
    glm::vec2 v;
    int vIndex = 0;
    char *currentNum = reinterpret_cast<char *>(std::malloc(32));
    int currentNumIndex = 0;
    for (const auto &c : line) {
        if (c == ' ') {
            currentNum[currentNumIndex] = '\0';
            v[vIndex] = std::strtof(currentNum, nullptr);
            currentNumIndex = 0;
            vIndex++;
            continue;
        }
        currentNum[currentNumIndex++] = c;
    }
    currentNum[currentNumIndex] = '\0';
    v[vIndex] = std::strtof(currentNum, nullptr);
    list.push_back(v);
}

void parseVec3(const std::string &fileName, unsigned long lineNumber, const std::string &line,
               std::vector<glm::vec3> &list) {
    glm::vec3 v;
    int vIndex = 0;
    char *currentNum = reinterpret_cast<char *>(std::malloc(32));
    int currentNumIndex = 0;
    for (const auto &c : line) {
        if (c == ' ') {
            currentNum[currentNumIndex] = '\0';
            v[vIndex] = std::strtof(currentNum, nullptr);
            currentNumIndex = 0;
            vIndex++;
            continue;
        }
        currentNum[currentNumIndex++] = c;
    }
    currentNum[currentNumIndex] = '\0';
    v[vIndex] = std::strtof(currentNum, nullptr);
    list.push_back(v);
}

void parseFace(const std::string &fileName, unsigned long lineNumber, const std::string &line, std::vector<Face> &faces,
               PerformanceCounter *pc) {
    std::string l = line.substr(2);

    std::vector<FaceVertex> faceVertices = {};
    char *currentNum = reinterpret_cast<char *>(std::malloc(32));
    int currentNumIndex = 0;
    int currentCoordinate = 0;
    FaceVertex currentVertex = {};
    for (auto &c : l) {
        if (c == ' ') {
            currentNum[currentNumIndex] = '\0';
            switch (currentCoordinate) {
            case 0:
                currentVertex.vertexIndex = std::strtoul(currentNum, nullptr, 10);
                break;
            case 1:
                currentVertex.textureCoordinateIndex = std::strtoul(currentNum, nullptr, 10);
                break;
            case 2:
                currentVertex.normalIndex = std::strtoul(currentNum, nullptr, 10);
                break;
            }
            faceVertices.push_back(currentVertex);
            currentVertex.vertexIndex = 0;
            currentVertex.textureCoordinateIndex = 0;
            currentVertex.normalIndex = 0;
            currentCoordinate = 0;
            currentNumIndex = 0;
            continue;
        }

        if (c == '/') {
            currentNum[currentNumIndex] = '\0';
            switch (currentCoordinate) {
            case 0:
                currentVertex.vertexIndex = std::strtoul(currentNum, nullptr, 10);
                break;
            case 1:
                currentVertex.textureCoordinateIndex = std::strtoul(currentNum, nullptr, 10);
                break;
            case 2:
                currentVertex.normalIndex = std::strtoul(currentNum, nullptr, 10);
                break;
            }
            currentCoordinate++;
            currentNumIndex = 0;
            continue;
        }

        currentNum[currentNumIndex++] = c;
    }

    currentNum[currentNumIndex] = '\0';
    switch (currentCoordinate) {
    case 0:
        currentVertex.vertexIndex = std::strtoul(currentNum, nullptr, 10);
        break;
    case 1:
        currentVertex.textureCoordinateIndex = std::strtoul(currentNum, nullptr, 10);
        break;
    case 2:
        currentVertex.normalIndex = std::strtoul(currentNum, nullptr, 10);
        break;
    }
    faceVertices.push_back(currentVertex);

    Face face = {faceVertices};
    faces.push_back(face);

    std::free(currentNum);
}

unsigned int Model::loadRawModelFromFile(const std::string &fileName, RawModel &model) {
    TIME_SCOPE_NAME("fromFile");

    if (fileName.find(".obj") == std::string::npos) {
        std::cerr << fileName << " is not an obj file" << std::endl;
        return 1;
    }

    std::ifstream modelStream(fileName, std::ios::in);

    if (!modelStream.is_open()) {
        std::cerr << "Could not open " << fileName << std::endl;
        return 1;
    }

    model.meshes = {};
    model.materials = {};

    RawMesh globalMesh = {};
    std::vector<Face> faces = {};

    {
        auto pc = PerformanceCounter();
        TIME_SCOPE_NAME("readLines");
        std::string l;
        int lineNumber = 0;
        while (std::getline(modelStream, l)) {
            lineNumber++;
            trim(l);
            if (l.empty()) {
                continue;
            }

            if (l[0] == '#') {
                continue;
            }

            if (l[0] == 'm' && l[1] == 't' && l[2] == 'l') {
                auto t = Timer(&pc, "materialLib");
                parseMaterialLib(fileName, lineNumber, l, model.materials);
            } else if (l[0] == 'o') {
                auto t = Timer(&pc, "object");
                if (!globalMesh.name.empty()) {
                    auto mesh = createIndicesFromFaces(globalMesh, faces);
                    model.meshes.push_back(mesh);
                    faces.clear();
                }
                globalMesh.name = l.substr(2);
            } else if (l[0] == 'v' && l[1] == 'n') {
                auto t = Timer(&pc, "normal");
                parseVec3(fileName, lineNumber, l.substr(3), globalMesh.normals);
            } else if (l[0] == 'v' && l[1] == 't') {
                auto t = Timer(&pc, "textureCoordinate");
                parseVec2(fileName, lineNumber, l.substr(3), globalMesh.uvs);
            } else if (l[0] == 'v') {
                auto t = Timer(&pc, "vertex");
                parseVec3(fileName, lineNumber, l.substr(2), globalMesh.vertices);
            } else if (l[0] == 'f') {
                auto t = Timer(&pc, "face");
                parseFace(fileName, lineNumber, l, faces, &pc);
            } else if (l[0] == 'u' && l[1] == 's' && l[2] == 'e') {
                auto t = Timer(&pc, "materialLibUsage");
                const int materialNameOffset = 7;
                globalMesh.material = model.materials[l.substr(materialNameOffset)];
            } else if (l[0] == 's') {
                // ignore this for now
            } else {
                std::cout << "Could not parse line " << lineNumber << " in " << fileName << ": " << l << std::endl;
            }
        }
        for (auto &dp : pc.dataPoints) {
            std::cout << dp.first << ": " << dp.second._sum << "ms\n";
        }
    }

    if (faces.empty()) {
        return 1;
    }

    auto mesh = createIndicesFromFaces(globalMesh, faces);
    model.meshes.push_back(mesh);
    return 0;
}
