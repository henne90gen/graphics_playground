#pragma once

#include <string>
#include <vector>
#include <map>
#include <glm/ext.hpp>

namespace ModelLoader {
    struct Material {
        std::string name;

        std::string diffuseTextureMap;
    };

    struct Mesh {
        std::string name;

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> textureCoordinates;
        std::vector<glm::ivec3> indices;

        bool hasMaterial;
        Material material;
    };

    struct Model {
        std::vector<Mesh> meshes;
        std::map<std::string, Material> materials;
    };

    unsigned int fromFile(const std::string &fileName, Model &model);

    unsigned int fromFileContent(const std::string &fileName, const std::vector<std::string> &lines, Model &model);
}
