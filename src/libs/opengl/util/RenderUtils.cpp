#include "RenderUtils.h"

#include <vector>

std::shared_ptr<VertexArray> createCubeVA(const std::shared_ptr<Shader> &shader) {
    static std::vector<glm::vec3> vertices = {
          // back
          // 0
          {-0.5F, -0.5F, -0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, 0, -1.0},
          // 1
          {-0.5F, 0.5F, -0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, 0, -1.0},
          // 2
          {0.5F, 0.5F, -0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, 0, -1.0},
          // 3
          {0.5F, -0.5F, -0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, 0, -1.0},

          // front
          // 4
          {-0.5F, -0.5F, 0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, 0, 1.0},
          // 5
          {0.5F, -0.5F, 0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, 0, 1.0},
          // 6
          {0.5F, 0.5F, 0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, 0, 1.0},
          // 7
          {-0.5F, 0.5F, 0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, 0, 1.0},

          // left
          // 8
          {-0.5F, -0.5F, -0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {-1.0, 0, 0},
          // 9
          {-0.5F, -0.5F, 0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {-1.0, 0, 0},
          // 10
          {-0.5F, 0.5F, 0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {-1.0, 0, 0},
          // 11
          {-0.5F, 0.5F, -0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {-1.0, 0, 0},

          // right
          // 12
          {0.5F, -0.5F, 0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {1.0, 0, 0},
          // 13
          {0.5F, -0.5F, -0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {1.0, 0, 0},
          // 14
          {0.5F, 0.5F, -0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {1.0, 0, 0},
          // 15
          {0.5F, 0.5F, 0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {1.0, 0, 0},

          // top
          // 16
          {-0.5F, 0.5F, 0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, 1.0, 0},
          // 17
          {0.5F, 0.5F, 0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, 1.0, 0},
          // 18
          {0.5F, 0.5F, -0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, 1.0, 0},
          // 19
          {-0.5F, 0.5F, -0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, 1.0, 0},

          // bottom
          // 20
          {-0.5F, -0.5F, 0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, -1.0, 0},
          // 21
          {-0.5F, -0.5F, -0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, -1.0, 0},
          // 22
          {0.5F, -0.5F, -0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, -1.0, 0},
          // 23
          {0.5F, -0.5F, 0.5F}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, -1.0, 0},
    };
    static std::vector<glm::ivec3> indices = {
          // front
          {0, 1, 2}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, 2, 3}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)

          // back
          {4, 5, 6}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {4, 6, 7}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)

          // left
          {8, 9, 10},  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {8, 10, 11}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)

          // right
          {12, 13, 14}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {12, 14, 15}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)

          // top
          {16, 17, 18}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {16, 18, 19}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)

          // bottom
          {20, 21, 22}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {20, 22, 23}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    };

    auto result = std::make_shared<VertexArray>(shader);
    BufferLayout layout = {
          {ShaderDataType::Float3, "a_Position"},
          {ShaderDataType::Float3, "a_Normal"},
    };
    std::shared_ptr<VertexBuffer> vb = std::make_shared<VertexBuffer>(vertices, layout);
    result->addVertexBuffer(vb);
    std::shared_ptr<IndexBuffer> ib = std::make_shared<IndexBuffer>(indices);
    result->setIndexBuffer(ib);

    return result;
}

void Sphere::append(std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &uvs,
                    std::vector<glm::ivec3> &indices) const {
    float sectorStep = glm::two_pi<float>() / static_cast<float>(sectorCount);
    float stackStep = glm::pi<float>() / static_cast<float>(stackCount);

    for (int i = 0; i <= stackCount; i++) {
        float stackAngle = glm::half_pi<float>() - static_cast<float>(i) * stackStep; // starting from pi/2 to -pi/2
        float xy = glm::cos(stackAngle);
        float z = glm::sin(stackAngle);

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; j++) {
            float sectorAngle = static_cast<float>(j) * sectorStep;
            float x = xy * glm::cos(sectorAngle);
            float y = xy * glm::sin(sectorAngle);
            vertices.emplace_back(x, y, z);

            // center of sphere is (0,0,0), thus the vertex coordinate is also the normal
            normals.emplace_back(x, y, z);

            float s = static_cast<float>(j) / static_cast<float>(sectorCount);
            float t = static_cast<float>(i) / static_cast<float>(stackCount);
            uvs.emplace_back(s, t);
        }
    }

    // stackCount * sectorCount * 2 - sectorCount * 2
    // (stackCount - 1) * sectorCount * 2

    int k1 = 0;
    int k2 = 0;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1); // beginning of current stack
        k2 = k1 + sectorCount + 1;  // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0) {
                indices.emplace_back(k1, k2, k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1)) {
                indices.emplace_back(k1 + 1, k2, k2 + 1);
            }
        }
    }
}

std::shared_ptr<VertexArray> Sphere::createVA(const std::shared_ptr<Shader> &shader) const {
    std::vector<glm::vec3> vertices = {};
    std::vector<glm::vec3> normals = {};
    std::vector<glm::vec2> uvs = {};
    std::vector<glm::ivec3> indices = {};

    append(vertices, normals, uvs, indices);

    auto vertexData = std::vector<float>(vertices.size() * 8);
#pragma omp parallel for
    for (int i = 0; i < vertices.size(); i++) {
        vertexData[i * 8 + 0] = vertices[i].x;
        vertexData[i * 8 + 1] = vertices[i].y;
        vertexData[i * 8 + 2] = vertices[i].z;
        vertexData[i * 8 + 3] = normals[i].x;
        vertexData[i * 8 + 4] = normals[i].y;
        vertexData[i * 8 + 5] = normals[i].z;
        vertexData[i * 8 + 6] = uvs[i].x;
        vertexData[i * 8 + 7] = uvs[i].y;
    }

    auto array = std::make_shared<VertexArray>(shader);
    BufferLayout layout = {
          {ShaderDataType::Float3, "a_Position"},
          {ShaderDataType::Float3, "a_Normal"},
          {ShaderDataType::Float2, "a_Uv"},
    };
    std::shared_ptr<VertexBuffer> vb = std::make_shared<VertexBuffer>(vertexData, layout);
    array->addVertexBuffer(vb);
    std::shared_ptr<IndexBuffer> ib = std::make_shared<IndexBuffer>(indices);
    array->setIndexBuffer(ib);
    return array;
}

std::shared_ptr<VertexArray> createQuadVA(const std::shared_ptr<Shader> &shader, const glm::vec2 &scale) {
    glm::vec2 uvMin = {0.0F, 0.0F};
    glm::vec2 uvMax = {1.0F, 1.0F};
    std::vector<float> vertices = {
          -0.5F * scale.x, -0.5F * scale.y, 0.0, uvMin.x, uvMin.y, 0.0F, 0.0F, 1.0F, //
          0.5F * scale.x,  -0.5F * scale.y, 0.0, uvMax.x, uvMin.y, 0.0F, 0.0F, 1.0F, //
          0.5F * scale.x,  0.5F * scale.y,  0.0, uvMax.x, uvMax.y, 0.0F, 0.0F, 1.0F, //
          -0.5F * scale.x, 0.5F * scale.y,  0.0, uvMin.x, uvMax.y, 0.0F, 0.0F, 1.0F, //
    };

    auto result = std::make_shared<VertexArray>(shader);
    BufferLayout bufferLayout = {
          {ShaderDataType::Float3, "a_Position"},
          {ShaderDataType::Float2, "a_UV"},
          {ShaderDataType::Float3, "a_Normal"},
    };
    auto buffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    result->addVertexBuffer(buffer);

    std::vector<glm::ivec3> indices = {
          {0, 1, 2}, //
          {0, 2, 3}, //
    };
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    result->setIndexBuffer(indexBuffer);

    return result;
}

std::shared_ptr<VertexArray> createBoundingBoxVA(const std::shared_ptr<Shader> &shader) {
    auto result = std::make_shared<VertexArray>(shader);

    std::vector<glm::vec3> vertices = {
          {-0.5F, -0.5F, -0.5F}, // 0 - - -
          {0.5F, -0.5F, -0.5F},  // 1 + - -
          {-0.5F, 0.5F, -0.5F},  // 2 - + -
          {0.5F, 0.5F, -0.5F},   // 3 + + -
          {-0.5F, -0.5F, 0.5F},  // 4 - - +
          {0.5F, -0.5F, 0.5F},   // 5 + - +
          {-0.5F, 0.5F, 0.5F},   // 6 - + +
          {0.5F, 0.5F, 0.5F},    // 7 + + +
    };

    BufferLayout layout = {{ShaderDataType::Float3, "position"}};
    auto vb = std::make_shared<VertexBuffer>(vertices, layout);
    result->addVertexBuffer(vb);

    std::vector<unsigned int> indices = {
          0, 1, 5, 4, 0,    // bottom layer
          2, 3, 7, 6, 2,    // top layer
          3, 1, 5, 7, 6, 4, // remaining edges
    };

    auto ib = std::make_shared<IndexBuffer>(indices);
    result->setIndexBuffer(ib);

    return result;
}
