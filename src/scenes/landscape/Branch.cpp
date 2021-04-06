#include "Branch.h"

const std::hash<std::string> position_hash;
float hashrand(int i) { return static_cast<float>(position_hash(std::to_string(i)) % 1000) / 1000.0F; }

std::mt19937 randomEngine;
std::uniform_int_distribution<int> distribution = std::uniform_int_distribution<int>(0, 100000);

int randInt() { return distribution(randomEngine); }

void Branch::grow(const TreeSettings &settings, double feed) {

    radius = sqrt(area / glm::pi<float>()); // Current Radius

    if (leaf) {
        length += cbrt(feed);      // Grow in Length
        feed -= cbrt(feed) * area; // Reduce Feed
        area += feed / length;     // Grow In Area

        // Split Condition
        if (length > splitsize * exp(-settings.splitdecay * static_cast<float>(depth))) {
            split(settings); // Split Behavior
        }

        return;
    }

    double pass = settings.passratio;

    if (settings.conservearea) {
        // Feedback Control for Area Conservation
        pass = (A->area + B->area) / (A->area + B->area + area);
    }

    area += pass * feed / length; // Grow in Girth
    feed *= (1.0 - pass);         // Reduce Feed

    if (feed < 1E-5) {
        return; // Prevent Over-Branching
    }

    A->grow(settings, feed * ratio); // Grow Children
    B->grow(settings, feed * (1.0 - ratio));
}

void Branch::split(const TreeSettings &settings) {

    leaf = false;

    // Add Child Branches
    A = new Branch(this, false);
    B = new Branch(this, false);
    A->ID = 2 * ID + 0; // Every Leaf ID is Unique (because binary!)
    B->ID = 2 * ID + 1;

    /*  Ideal Growth Direction:
          Perpendicular to direction with highest leaf density! */

    glm::vec3 D = leafdensity(settings, settings.localdepth); // Direction of Highest Density
    glm::vec3 N = glm::normalize(glm::cross(dir, D));         // Normal Vector
    glm::vec3 M = -1.0F * N;                                  // Reflection

    float flip = (randInt() % 2 == 0) ? 1.0 : -1.0; // Random Direction Flip
    A->dir = glm::normalize(glm::mix(flip * spread * N, dir, ratio));
    B->dir = glm::normalize(glm::mix(flip * spread * M, dir, 1.0 - ratio));
}

glm::vec3 Branch::leafdensity(const TreeSettings &settings, int searchdepth) {

    // Random Vector! (for noise)
    glm::vec3 r = glm::vec3(randInt() % 100, randInt() % 100, randInt() % 100) / glm::vec3(100) - glm::vec3(0.5);

    if (depth == 0) {
        return r;
    }

    /*
      General Idea: Branches grow away from areas with a high leaf density!

      Therefore, if we can compute a vector that points towards the area with
      the locally highest leaf density, we can use that to compute our normal
      for branching.

      Locally high density is determined by descending the tree to some maximum
      search depth (finding an ancestor node), and computing some leaf-density
      metric over the descendant node leaves. This is implemented recursively.

      Metric 1: Uniform Weights in Space.
        Problem: Causes strange spiral artifacts at high-search depths, because it
          computes the average leaf position of the entire tree. Therefore,
          the tree grows in a strange way, away from the center.

      Metric 2: Distance weighted average position (i.e. relative vector)
        Problem: This causes strange cone artifacts when using a sufficiently large
          search depth. This is also more expensive to compute, and wonky because
          we compute the distance-weighted average distance (what?? exactly).

      Since both metrics give similar results at a small search-depth (e.g. 2),
      meaning we only search locally, I will use the one that is simpler to compute.
      That is Method 1.

      I did throw in a weighting by the branch ratio though, just because I can.
      That means that the tree should tend to grow away from branches with the most
      growth potential.

    */

    Branch *C = this;                            // Ancestor node
    auto rel = glm::vec3(0);                     // Relative position to start node
    while (C->depth > 0 && searchdepth-- >= 0) { // Descend tree
        rel += C->length * C->dir;               // Add relative position
        C = C->P;                                // Move to parent
    }

    std::function<glm::vec3(Branch *)> leafaverage = [&](Branch *b) -> glm::vec3 {
        if (b->leaf) {
            return b->length * b->dir;
        }
        return b->length * b->dir + ratio * leafaverage(b->A) + (1.0f - ratio) * leafaverage(b->B);
    };

    // Average relative to ancestor, shifted by rel ( + Noise )
    return settings.directedness * glm::normalize(leafaverage(C) - rel) + (1.0F - settings.directedness) * r;
}

void Tree::construct(std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &uvs,
                     std::vector<glm::ivec3> &indices) {
    // Basically Add Lines for the Tree!
    std::function<void(Branch *, glm::vec3)> addBranch = [&](Branch *b, glm::vec3 p) {
        glm::vec3 start = p;
        glm::vec3 end = p + glm::vec3(b->length * settings.treescale[0]) * b->dir;

        // Get Some Normal Vector
        glm::vec3 x = glm::normalize(b->dir + glm::vec3(1.0, 1.0, 1.0));
        glm::vec4 n = glm::vec4(glm::normalize(glm::cross(b->dir, x)), 1.0);

        // Add the Correct Number of Indices
        glm::mat4 r = glm::rotate(glm::mat4(1.0), glm::pi<float>() / static_cast<float>(settings.ringsize), b->dir);

        // Index Buffer
        int _b = positions.size();

        // GL TRIANGLES
        for (int i = 0; i < settings.ringsize; i++) {
            // Bottom Triangle
            indices.emplace_back(                             //
                  _b + i * 2 + 0,                             //
                  _b + (i * 2 + 2) % (2 * settings.ringsize), //
                  _b + i * 2 + 1                              //
            );

            // Upper Triangle
            indices.emplace_back(                             //
                  _b + (i * 2 + 2) % (2 * settings.ringsize), //
                  _b + (i * 2 + 3) % (2 * settings.ringsize), //
                  _b + i * 2 + 1                              //
            );
        }

        for (int i = 0; i < settings.ringsize; i++) {
            auto percentAround = static_cast<float>(i) / static_cast<float>(settings.ringsize);

            positions.push_back(start + b->radius * settings.treescale[1] * glm::vec3(n));
            normals.emplace_back(n);
            uvs.emplace_back(0.0F, percentAround);
            n = r * n;

            positions.push_back(end + settings.taper * b->radius * settings.treescale[1] * glm::vec3(n));
            normals.emplace_back(n);
            uvs.emplace_back(1.0F, percentAround);
            n = r * n;
        }

        if (b->leaf) {
            return;
        }

        addBranch(b->A, end);
        addBranch(b->B, end);
    };

    // Recursive add Branches
    addBranch(root, glm::vec3(0.0));
}

// Construct Leaf Particle System from Tree Data
void Tree::addLeaves(std::vector<glm::mat4> &p) const {
    p.clear();

    // Explore the Tree and Add Leaves!
    std::function<void(Branch *, glm::vec3)> addLeaf = [&](Branch *b, glm::vec3 pos) {
        if (!b->leaf) {
            // recurse
            glm::vec3 end = pos + glm::vec3(b->length * settings.treescale[0]) * b->dir;
            addLeaf(b->A, end);
            addLeaf(b->B, end);
            return;
        }

        for (int i = 0; i < settings.leafcount; i++) {
            // Hashed Random Displace
            glm::vec3 d = glm::vec3(hashrand(b->ID + i), hashrand(b->ID + i + settings.leafcount),
                                    hashrand(b->ID + i + 2 * settings.leafcount)) -
                          glm::vec3(0.5);
            d *= glm::vec3(settings.leafspread[0], settings.leafspread[1], settings.leafspread[2]);

            // Rotate and Scale
            glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), pos + d);
            model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0, 1.0, 0.0));

            p.push_back(glm::scale(model, glm::vec3(settings.leafsize)));
        }
    };

    addLeaf(root, glm::vec3(0.0));
}

Tree *Tree::create(const TreeSettings &settings) {
    randomEngine = std::mt19937(settings.randomSeed);
    auto *root = new Branch(0.6F, 0.45F, 2.5F);
    for (int i = 0; i < 100; i++) {
        root->grow(settings, settings.growthrate);
    }
    return new Tree(settings, root);
}
