#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "gl/VertexArray.h"

std::shared_ptr<VertexArray> createCubeVA(const std::shared_ptr<Shader> &shader);

/**
 * Copied from http://www.songho.ca/opengl/gl_sphere.html
 * Creates a unit sphere with center at (0,0,0) and radius 1.0
 * @param sectorCount "rings" around the sphere
 * @param stackCount "strips" from top to bottom of the sphere
 * @return
 */
std::shared_ptr<VertexArray> createSphereVA(const std::shared_ptr<Shader> &shader, int sectorCount = 36,
                                            int stackCount = 18);

/**
 * Copied from http://www.songho.ca/opengl/gl_sphere.html
 * Creates a unit sphere with center at (0,0,0) and radius 1.0
 * @param sectorCount "rings" around the sphere
 * @param stackCount "strips" from top to bottom of the sphere
 * @return
 */
void appendSphere(std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &uvs,
                  std::vector<glm::ivec3> &indices, int sectorCount = 36, int stackCount = 18);

std::shared_ptr<VertexArray> createQuadVA(const std::shared_ptr<Shader> &shader, const glm::vec2 &scale = {1.0F, 1.0F});

std::shared_ptr<VertexArray> createBoundingBoxVA(const std::shared_ptr<Shader> &shader);
