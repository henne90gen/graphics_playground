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
    unsigned long characterCode;
    Texture texture;
    glm::ivec2 dimensions;
    glm::ivec2 offset;
    long advance;
    long requestedCharacterHeight;
};

class FontDemo : public Scene {
public:
    FontDemo(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "FontDemo") {};

    ~FontDemo() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    VertexArray *vertexArray;
    Shader *shader;

    std::vector<Character> characters = {};

    FT_Library library = nullptr;
    FT_Face face = nullptr;

    void loadFont(std::string &fontPath, int characterHeight);

    Character loadCharacter(unsigned long characterCode, int characterHeight);

    void loadAlphabet(int characterHeight);

    void renderCharacter(const Character &character, const glm::vec2 &translation, float scale) const;

    std::vector<std::string> getFontPaths();

    void renderAlphabet(const glm::vec2 &translation, float zoom) const;

    void renderText(std::string &text, const glm::vec2 &translation, float zoom);
};

void showSettings(const std::vector<std::string> &fontPaths, glm::vec3 &color, glm::vec2 &translation, float &zoom,
                  unsigned int &characterResolution, unsigned int &selectedFontIndex, FT_Face &face);

void showFontInfo(FT_Face &face);

bool settingsHaveChanged(int characterHeight, int selectedFontIndex);