#include "RainSimulation.h"

#include "util/AssertUtils.h"

void setHeightMapValue(HeightMap &heightMap, int x, int z, float value) {
    unsigned long safeZ = std::min(std::max(z, 0), static_cast<int>(heightMap.height) - 1);
    unsigned long safeX = std::min(std::max(x, 0), static_cast<int>(heightMap.width) - 1);
    unsigned long index = safeZ * heightMap.width + safeX;
    ASSERT(index < heightMap.data.size());
    heightMap.data[index] = value;
}

float getHeightMapValue(const HeightMap &heightMap, int x, int z) {
    unsigned long safeZ = std::min(std::max(z, 0), static_cast<int>(heightMap.height) - 1);
    unsigned long safeX = std::min(std::max(x, 0), static_cast<int>(heightMap.width) - 1);
    unsigned long index = safeZ * heightMap.width + safeX;
    ASSERT(index < heightMap.data.size());
    return heightMap.data[index];
}

void simulateRaindrop(HeightMap &heightMap, std::mt19937 randomGenerator,
                      std::uniform_real_distribution<double> randomDistribution, const SimulationParams &params,
                      Raindrop &raindrop) {
    float Kq = params.Kq;
    float Kw = params.Kw;
    float Kr = params.Kr;
    float Kd = params.Kd;
    float Ki = params.Ki;
    float minSlope = params.minSlope;
    float g = params.g;

#define DEPOSIT_AT(X, Z, W)                                                                                            \
    {                                                                                                                  \
        float delta = ds * (W);                                                                                        \
        float newH = getHeightMapValue(heightMap, (X), (Z)) + delta;                                                   \
        setHeightMapValue(heightMap, (X), (Z), newH);                                                                  \
    }

#if 1
#define DEPOSIT(H)                                                                                                     \
    DEPOSIT_AT(pi.x, pi.y, (1 - f.x) * (1 - f.y))                                                                      \
    DEPOSIT_AT(pi.x + 1, pi.y, f.x * (1 - f.y))                                                                        \
    DEPOSIT_AT(pi.x, pi.y + 1, (1 - f.x) * f.y)                                                                        \
    DEPOSIT_AT(pi.x + 1, pi.y + 1, f.x * f.y)                                                                          \
    (H) += ds;
#else
#define DEPOSIT(H)                                                                                                     \
    DEPOSIT_AT(pi.x, pi.y, 0.25f)                                                                                      \
    DEPOSIT_AT(pi.x + 1, pi.y, 0.25f)                                                                                  \
    DEPOSIT_AT(pi.x, pi.y + 1, 0.25f)                                                                                  \
    DEPOSIT_AT(pi.x + 1, pi.y + 1, 0.25f)                                                                              \
    (H) += ds;
#endif

    raindrop.velocity = 0.0F;
    raindrop.water = 1.0F;
    raindrop.sediment = 0.0F;
    raindrop.path.clear();

    glm::vec2 p = {raindrop.startingPosition.x, raindrop.startingPosition.y};
    glm::ivec2 pi = {static_cast<int>(p.x), static_cast<int>(p.y)};
    glm::vec2 f = p - glm::vec2(pi.x, pi.y);
    glm::vec2 d = {0.0F, 0.0F};
    float h00 = getHeightMapValue(heightMap, pi.x, pi.y);
    float h10 = getHeightMapValue(heightMap, pi.x + 1, pi.y);
    float h01 = getHeightMapValue(heightMap, pi.x, pi.y + 1);
    float h11 = getHeightMapValue(heightMap, pi.x + 1, pi.y + 1);
    float h = h00;

    const int maxPathLength = heightMap.width * 4;
    for (unsigned int i = 0; i < maxPathLength; i++) {
        raindrop.path.emplace_back(p.x, h, p.y);

        glm::vec2 gradient = {
              (h10 - h00) * (1 - f.y) + (h11 - h01) * f.y, //
              (h01 - h00) * (1 - f.x) + (h11 - h10) * f.x  //
        };
        d = d * Ki - gradient * (1.0F - Ki);

        float dl = glm::length(d);
        if (dl <= FLT_EPSILON) {
            // pick random dir
            double a = randomDistribution(randomGenerator) * glm::two_pi<double>();
            d = {std::cos(a), std::sin(a)};
        }

        d = glm::normalize(d);

        glm::vec2 np = p + d;
        glm::ivec2 ni = {std::floor(np.x), std::floor(np.y)};
        glm::vec2 nf = np - glm::vec2(ni.x, ni.y);

        float nh00 = getHeightMapValue(heightMap, ni.x, ni.y);
        float nh01 = getHeightMapValue(heightMap, ni.x, ni.y + 1);
        float nh10 = getHeightMapValue(heightMap, ni.x + 1, ni.y);
        float nh11 = getHeightMapValue(heightMap, ni.x + 1, ni.y + 1);

        float nh0 = nh00 * (1 - nf.x) + nh10 * nf.x;
        float nh1 = nh01 * (1 - nf.x) + nh11 * nf.x;
        float nh = nh0 * (1 - nf.y) + nh1 * nf.y;

        // if higher than current, try to deposit sediment up to neighbour height
        if (nh >= h) {
            float ds = (nh - h) + 0.001F;

            if (ds >= raindrop.sediment) {
                // deposit all sediment and stop
                ds = raindrop.sediment;
                DEPOSIT(h)
                raindrop.sediment = 0.0F;
                break;
            }

            DEPOSIT(h)
            raindrop.sediment -= ds;
            raindrop.velocity = 0.0F;
        }

        // compute transport capacity
        float dh = h - nh;
        float slope = dh;
        //         float slope = dh / sqrtf(dh * dh + 1);

        float carryCapacity = std::max(slope, minSlope) * raindrop.velocity * raindrop.water * Kq;
        float ds = raindrop.sediment - carryCapacity;
        if (ds >= 0) {
            // deposit
            ds *= Kd;
            // ds=minval(ds, 1.0f);

            DEPOSIT(dh)
            raindrop.sediment -= ds;
        } else {
            // erode
            ds *= -Kr;
            ds = std::min(ds, dh * 0.99f);

#define ERODE(X, Z, W)                                                                                                 \
    {                                                                                                                  \
        float delta = ds * (W);                                                                                        \
        float newH = getHeightMapValue(heightMap, (X), (Z)) - delta;                                                   \
        setHeightMapValue(heightMap, (X), (Z), newH);                                                                  \
    }
#if 1
            for (int z = pi.y - 1; z <= pi.y + 2; z++) {
                float zo = static_cast<float>(z) - p.y;
                float zo2 = zo * zo;

                for (int x = pi.x - 1; x <= pi.x + 2; x++) {
                    float xo = static_cast<float>(x) - p.x;

                    float w = 1 - (xo * xo + zo2) * 0.25F;
                    if (w <= 0) {
                        continue;
                    }
                    w *= 0.1591549430918953f;

                    ERODE(x, z, w)
                }
            }
#else
            ERODE(pi.x, pi.y, (1 - f.x) * (1 - f.y))
            ERODE(pi.x + 1, pi.y, f.x * (1 - f.y))
            ERODE(pi.x, pi.y + 1, (1 - f.x) * f.y)
            ERODE(pi.x + 1, pi.y + 1, f.x * f.y)
#endif

#undef ERODE

            dh -= ds;
            raindrop.sediment += ds;
        }

        // move to the neighbour
        raindrop.velocity = std::sqrt(raindrop.velocity * raindrop.velocity + g * dh);
        raindrop.water *= 1 - Kw;

        p = np;
        pi = ni;
        f = nf;

        h = nh;
        h00 = nh00;
        h10 = nh10;
        h01 = nh01;
        h11 = nh11;
    }
}
