#include "RenderUtils.h"

#include <vector>

std::shared_ptr<VertexArray> createCubeVA(std::shared_ptr<Shader> shader) {
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
