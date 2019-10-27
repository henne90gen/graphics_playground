#include "RenderUtils.h"

#include <vector>

std::shared_ptr<VertexArray> createCubeVA(const std::shared_ptr<Shader> &shader) {
    std::vector<glm::vec3> vertices = {
          // back
          {-1.0F, -1.0F, -1.0F}, // 0
          {1.0F, -1.0F, -1.0F},  // 1
          {1.0F, 1.0F, -1.0F},   // 2
          {-1.0F, 1.0F, -1.0F},  // 3

          // front
          {-1.0F, -1.0F, 1.0F}, // 4
          {1.0F, -1.0F, 1.0F},  // 5
          {1.0F, 1.0F, 1.0F},   // 6
          {-1.0F, 1.0F, 1.0F},  // 7
    };
    std::vector<glm::ivec3> indices = {
          // front
          {0, 1, 2}, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
          {0, 2, 3}, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

          // back
          {4, 5, 6}, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
          {4, 6, 7}, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

          // right
          {5, 1, 2}, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
          {5, 2, 6}, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

          // left
          {0, 4, 7}, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
          {0, 7, 3}, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

          // top
          {7, 6, 2}, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
          {7, 2, 3}, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

          // bottom
          {4, 5, 1}, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
          {4, 1, 0}, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    };

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

std::shared_ptr<VertexArray> createSphereVA(const std::shared_ptr<Shader> &shader, const int sectorCount,
                                            const int stackCount) {
    std::vector<glm::vec3> vertices = {};
    std::vector<glm::ivec3> indices = {};

    float sectorStep = glm::two_pi<float>() / (float)sectorCount;
    float stackStep = glm::pi<float>() / (float)stackCount;
    float sectorAngle, stackAngle;
    float x, y, z, xy;

    for (int i = 0; i <= stackCount; i++) {
        stackAngle = glm::half_pi<float>() - (float)i * stackStep; // starting from pi/2 to -pi/2
        xy = glm::cos(stackAngle);
        z = glm::sin(stackAngle);

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; j++) {
            sectorAngle = (float)j * sectorStep;
            x = xy * glm::cos(sectorAngle);
            y = xy * glm::sin(sectorAngle);
            vertices.emplace_back(x, y, z);
        }
    }

    int k1, k2;
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
