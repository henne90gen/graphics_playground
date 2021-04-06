#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <random>
#include <string>

/**
 * https://github.com/weigert/TinyEngine/tree/master/examples/6_Tree
 */

struct TreeSettings {
    int randomSeed = 1337;
    glm::vec2 treescale = {15.0F, 5.0F};

    int ringsize = 12;
    int leafcount = 2;
    float leafsize = 15.0F;
    float taper = 0.6F;
    glm::vec3 leafspread = {50.0F, 50.0F, 50.0F};

    float growthrate = 1.0F;
    float passratio = 0.3F;
    float splitdecay = 1E-2F;
    float directedness = 0.5F;
    int localdepth = 2;
    bool conservearea = true;

    void showGui() {
        if (ImGui::TreeNode("Tree Settings")) {
            ImGui::SliderInt("randomSeed", &randomSeed, 0, 1e+9);
            ImGui::DragFloat2("treescale", reinterpret_cast<float *>(&treescale), 0.01F);
            ImGui::SliderInt("ringsize", &ringsize, 3, 30);
            ImGui::SliderInt("leafcount", &leafcount, 0, 100);
            ImGui::DragFloat("leafsize", &leafsize, 0.01F);
            ImGui::DragFloat("taper", &taper, 0.001F);
            ImGui::DragFloat3("leafspread", reinterpret_cast<float *>(&leafspread), 0.1F);

            ImGui::DragFloat("growthrate", &growthrate, 0.001F);
            ImGui::DragFloat("passratio", &passratio, 0.001F);
            ImGui::DragFloat("splitdecay", &splitdecay, 0.001F);
            ImGui::DragFloat("directedness", &directedness, 0.01F);
            ImGui::SliderInt("localdepth", &localdepth, 0, 100);
            ImGui::Checkbox("conservearea", &conservearea);
            ImGui::TreePop();
        }
    }
};

struct Branch {
    // For Leaf Hashing
    int ID = 0;
    bool leaf = true;

    // Child A, B and Parent
    Branch *A = nullptr;
    Branch *B = nullptr;
    Branch *P = nullptr;

    // Parameters
    float ratio = 0.0F;
    float spread = 0.0F;
    float splitsize = 0.0F;
    int depth = 0;

    // Size / Direction Data
    glm::vec3 dir = glm::vec3(0.0, 1.0, 0.0);
    float length = 0.0F;
    float radius = 0.0F;
    float area = 0.1F;

    // Constructors
    explicit Branch(float r, float s, float ss) : ratio{r}, spread{s}, splitsize{ss} {};
    explicit Branch(Branch *b, bool root) : ratio{b->ratio}, spread{b->spread}, splitsize{b->splitsize} {
        if (root) {
            return;
        }

        depth = b->depth + 1;
        P = b; // Set Parent
    };

    ~Branch() {
        if (leaf) {
            return;
        }

        delete (A);
        delete (B);
    }

    void grow(const TreeSettings &settings, double feed);
    void split(const TreeSettings &settings);

    // Compute Direction to Highest Local Leaf Density
    glm::vec3 leafdensity(const TreeSettings &settings, int searchdepth);
};

struct Tree {
    const TreeSettings &settings;
    Branch *root = nullptr;

    explicit Tree(const TreeSettings &s, Branch *b) : settings(s), root(b) {}

    void construct(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &uvs,
                   std::vector<glm::ivec3> &indices);
    void addLeaves(std::vector<glm::mat4> &p) const;

    static Tree *create(const TreeSettings &settings);
};
