#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <gl/Texture.h>
#include <glm/ext.hpp>
#include <optional>
#include <string>
#include <unordered_map>

struct Character {
    char character;
    Texture texture;
    glm::ivec2 dimension;
    glm::ivec2 pixelOffset;
    long advance;
    long maxHeight;

    [[nodiscard]] glm::vec2 offset(float scale) const {
        auto off = glm::vec2();
        off.x = static_cast<float>(pixelOffset.x);
        off.x -= (static_cast<float>(maxHeight) - static_cast<float>(dimension.x)) / scale;
        off.y = static_cast<float>(dimension.y) - static_cast<float>(pixelOffset.y);
        off.y += (static_cast<float>(maxHeight) - static_cast<float>(dimension.y)) / scale;
        off.y *= -1.0F;
        off /= maxHeight;
        return off;
    }

    [[nodiscard]] glm::vec2 scale() const {
        return {
              static_cast<float>(dimension.x) / static_cast<float>(maxHeight),
              static_cast<float>(dimension.y) / static_cast<float>(maxHeight),
        };
    }
};

struct Text {
    FT_Library library = nullptr;
    FT_Face face = nullptr;
    std::unordered_map<char, Character> characters = {};

    void load(const std::string &fontPath, unsigned int characterHeight);
    void unload() const;

    std::optional<Character> character(char c);

  private:
    void loadAlphabet(unsigned int characterHeight);
    [[nodiscard]] std::optional<Character> loadCharacter(char character, unsigned int characterHeight) const;
};
