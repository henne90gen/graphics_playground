#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "opengl/VertexArray.h"

std::shared_ptr<VertexArray> createCubeVA(const std::shared_ptr<Shader> &shader);

/**
 * Copied from http://www.songho.ca/opengl/gl_sphere.html
 * @param shader
 * @param sectorCount "rings" around the sphere
 * @param stackCount "strips" from top to bottom of the sphere
 * @return
 */
std::shared_ptr<VertexArray> createSphereVA(const std::shared_ptr<Shader> &shader, int sectorCount = 36,
                                            int stackCount = 18);
