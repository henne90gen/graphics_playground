#include "GraphVis.h"

void GraphVis::setup() {
    shader = std::make_shared<Shader>("scenes/graph_vis/GraphVisVert.glsl",
                                      "scenes/graph_vis/GraphVisFrag.glsl");
    shader->bind();
}

void GraphVis::destroy() {}

void GraphVis::tick() {}
