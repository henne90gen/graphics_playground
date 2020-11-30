#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "scenes/Scene.h"

#include <functional>

#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/Texture.h"
#include "opengl/Shader.h"

struct Character {
    char character;
    Texture texture;
    glm::ivec2 dimension;
    glm::ivec2 offset;
    long advance;
    long maxHeight;
};

class FontDemo : public Scene {
public:
    explicit FontDemo(SceneData data) : Scene(data, "FontDemo") {};

    ~FontDemo() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;

    std::vector<Character> characters = {};

    FT_Library library = nullptr;
    FT_Face face = nullptr;

    void loadFont(std::string &fontPath, unsigned int characterHeight);

    Character loadCharacter(const char character, const unsigned int characterHeight);

    void loadAlphabet(unsigned int characterHeight);

    void renderCharacter(const Character &character, const glm::vec2 &translation) const;

    void renderAlphabet(const glm::vec2 &translation, float zoom) const;

    void renderText(std::string &text, const glm::vec2 &translation, float zoom);

    void setViewMatrix(const glm::vec2 &translation, float zoom) const;
};

void showSettings(std::vector<std::string> &fontPaths, glm::vec3 &color, glm::vec2 &translation, float &zoom,
                  unsigned int &characterResolution, unsigned int &selectedFontIndex, bool &shouldRenderAlphabet,
                  FT_Face &face);

void showFontInfo(FT_Face &face);

bool settingsHaveChanged(unsigned int characterHeight, unsigned int selectedFontIndex);

std::string toBits(long number);
