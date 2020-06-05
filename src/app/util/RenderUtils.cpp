#include "RenderUtils.h"

#include <vector>

std::shared_ptr<VertexArray> createCubeVA(const std::shared_ptr<Shader> &shader) {
    static std::vector<glm::vec3> vertices = {
          // back
          {-0.5F, -0.5F, -0.5F},
          {0, 0, -1.0}, // 0
          {-0.5F, 0.5F, -0.5F},
          {0, 0, -1.0}, // 1
          {0.5F, 0.5F, -0.5F},
          {0, 0, -1.0}, // 2
          {0.5F, -0.5F, -0.5F},
          {0, 0, -1.0}, // 3

          // front
          // 4
          {-0.5F, -0.5F, 0.5F},
          {0, 0, 1.0},
          // 5
          {0.5F, -0.5F, 0.5F},
          {0, 0, 1.0},
          // 6
          {0.5F, 0.5F, 0.5F},
          {0, 0, 1.0},
          // 7
          {-0.5F, 0.5F, 0.5F},
          {0, 0, 1.0},

          // left
          // 8
          {-0.5F, -0.5F, -0.5F},
          {1.0, 0, 0},
          // 9
          {-0.5F, -0.5F, 0.5F},
          {1.0, 0, 0},
          // 10
          {-0.5F, 0.5F, 0.5F},
          {1.0, 0, 0},
          // 11
          {-0.5F, 0.5F, -0.5F},
          {1.0, 0, 0},

          // right
          // 12
          {0.5F, -0.5F, 0.5F},
          {-1.0, 0, 0},
          // 13
          {0.5F, -0.5F, -0.5F},
          {-1.0, 0, 0},
          // 14
          {0.5F, 0.5F, -0.5F},
          {-1.0, 0, 0},
          // 15
          {0.5F, 0.5F, 0.5F},
          {-1.0, 0, 0},

          // top
          // 16
          {-0.5F, 0.5F, 0.5F},
          {0, 1.0, 0},
          // 17
          {0.5F, 0.5F, 0.5F},
          {0, 1.0, 0},
          // 18
          {0.5F, 0.5F, -0.5F},
          {0, 1.0, 0},
          // 19
          {-0.5F, 0.5F, -0.5F},
          {0, 1.0, 0},

          // bottom
          // 20
          {-0.5F, -0.5F, 0.5F},
          {0, -1.0, 0},
          // 21
          {-0.5F, -0.5F, -0.5F},
          {0, -1.0, 0},
          // 22
          {0.5F, -0.5F, -0.5F},
          {0, -1.0, 0},
          // 23
          {0.5F, -0.5F, 0.5F},
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

std::shared_ptr<VertexArray> createSphereVA(const std::shared_ptr<Shader> &shader, const int sectorCount,
                                            const int stackCount) {
    std::vector<glm::vec3> vertices = {};
    std::vector<glm::ivec3> indices = {};

    float sectorStep = glm::two_pi<float>() / static_cast<float>(sectorCount);
    float stackStep = glm::pi<float>() / static_cast<float>(stackCount);
    float sectorAngle;
    float stackAngle;
    float x;
    float y;
    float z;
    float xy;

    for (int i = 0; i <= stackCount; i++) {
        stackAngle = glm::half_pi<float>() - static_cast<float>(i) * stackStep; // starting from pi/2 to -pi/2
        xy = glm::cos(stackAngle);
        z = glm::sin(stackAngle);

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; j++) {
            sectorAngle = static_cast<float>(j) * sectorStep;
            x = xy * glm::cos(sectorAngle);
            y = xy * glm::sin(sectorAngle);
            vertices.emplace_back(x, y, z);
        }
    }

    int k1;
    int k2;
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

    auto array = std::make_shared<VertexArray>(shader);
    BufferLayout layout = {
          {ShaderDataType::Float3, "a_Position"},
    };
    std::shared_ptr<VertexBuffer> vb = std::make_shared<VertexBuffer>(vertices, layout);
    array->addVertexBuffer(vb);
    std::shared_ptr<IndexBuffer> ib = std::make_shared<IndexBuffer>(indices);
    array->setIndexBuffer(ib);
    return array;
}

std::shared_ptr<VertexArray> createQuadVA(const std::shared_ptr<Shader> &shader,
                                          const glm::vec2 &scale) {
    glm::vec2 uvMin = {0.0F, 0.0F};
    glm::vec2 uvMax = {1.0F, 1.0F};
    std::vector<float> vertices = {
          -0.5F * scale.x, -0.5F * scale.y, 0.0, uvMin.x, uvMin.y, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          0.5F * scale.x,  -0.5F * scale.y, 0.0, uvMax.x, uvMin.y, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          0.5F * scale.x,  0.5F * scale.y,  0.0, uvMax.x, uvMax.y, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          -0.5F * scale.x, 0.5F * scale.y,  0.0, uvMin.x, uvMax.y, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    };

    auto result = std::make_shared<VertexArray>(shader);
    BufferLayout bufferLayout = {
          {ShaderDataType::Float3, "a_Position"},
          {ShaderDataType::Float2, "a_UV"},
    };
    auto buffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    result->addVertexBuffer(buffer);

    std::vector<glm::ivec3> indices = {
          {0, 1, 2}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, 2, 3}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    };
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    result->setIndexBuffer(indexBuffer);

    return result;
}
