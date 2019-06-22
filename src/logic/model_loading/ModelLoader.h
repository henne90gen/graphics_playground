#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <glm/ext.hpp>

namespace ModelLoader {
    struct RawMaterial {
        std::string name;

        std::string diffuseTextureMap;
    };

    struct RawMesh {
        std::string name;

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> textureCoordinates;
        std::vector<glm::ivec3> indices;

        std::shared_ptr<RawMaterial> material;
    };

    struct RawModel {
        std::vector<RawMesh> meshes;
        std::map<std::string, std::shared_ptr<RawMaterial>> materials;
    };

    unsigned int fromFile(const std::string &fileName, std::shared_ptr<RawModel> &model);

    unsigned int fromFileContent(const std::string &fileName, const std::vector<std::string> &lines,
                                 std::shared_ptr<RawModel> &model);
}
