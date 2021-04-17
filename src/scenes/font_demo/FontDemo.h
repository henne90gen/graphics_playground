#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Scene.h"

#include <functional>

#include <Text.h>
#include <gl/Shader.h>
#include <gl/Texture.h>
#include <gl/VertexArray.h>
#include <gl/VertexBuffer.h>

class FontDemo : public Scene {
  public:
    explicit FontDemo() : Scene("FontDemo"){};

    ~FontDemo() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;

    Text t = {};

#if 0
        std::vector<Character> characters = {};

        FT_Library library = nullptr;
        FT_Face face = nullptr;

        void loadFont(std::string &fontPath, unsigned int characterHeight);
        Character loadCharacter(char character, unsigned int characterHeight);
        void loadAlphabet(unsigned int characterHeight);
#endif

    void renderCharacter(const Character &character, const glm::vec2 &translation) const;
    void renderAlphabet(const glm::vec2 &translation, float zoom);
    void renderText(std::string &text, const glm::vec2 &translation, float zoom);

    void setViewMatrix(const glm::vec2 &translation, float zoom) const;
};

void showSettings(std::vector<std::string> &fontPaths, glm::vec3 &color, glm::vec2 &translation, float &zoom,
                  unsigned int &characterResolution, unsigned int &selectedFontIndex, bool &shouldRenderAlphabet,
                  Text &t);

void showFontInfo(FT_Face &face);

bool settingsHaveChanged(unsigned int characterHeight, unsigned int selectedFontIndex);

std::string toBits(long number);
