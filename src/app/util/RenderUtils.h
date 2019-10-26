#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "opengl/VertexArray.h"

std::shared_ptr<VertexArray> createCubeVA(std::shared_ptr<Shader> shader);
