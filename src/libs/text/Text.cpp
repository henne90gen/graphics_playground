#include "Text.h"

#include <iostream>

void Text::load(const std::string &fontPath, unsigned int characterHeight) {
    if (library == nullptr) {
        const int error = FT_Init_FreeType(&library);
        if (error != 0) {
            std::cerr << "Could not initialize freetype" << std::endl;
            return;
        }
    }

    long faceIndex = 0;
    int error = FT_New_Face(library, fontPath.c_str(), faceIndex, &face);
    if (error == FT_Err_Unknown_File_Format) {
        std::cerr << "Corrupt font file" << std::endl;
        return;
    }
    if (error != 0) {
        std::cerr << "Could not read font file" << std::endl;
        return;
    }

    const int pixelWidth = 0;
    error = FT_Set_Pixel_Sizes(face, pixelWidth, characterHeight);
    if (error != 0) {
        std::cerr << "Could not set the character pixel size." << std::endl;
        return;
    }

    loadAlphabet(characterHeight);
}

void Text::loadAlphabet(unsigned int characterHeight) {
    characters.clear();
    const unsigned int firstCharacter = 32;
    const unsigned int lastCharacter = 127;
    for (unsigned int i = firstCharacter; i < lastCharacter; i++) {
        char c = static_cast<char>(i);
        std::optional<Character> characterOpt = loadCharacter(c, characterHeight);
        if (!characterOpt.has_value()) {
            continue;
        }
        characters[c] = characterOpt.value();
    }
}

std::optional<Character> Text::loadCharacter(char character, unsigned int characterHeight) const {
    int error = FT_Load_Char(face, character, FT_LOAD_RENDER); // NOLINT(hicpp-signed-bitwise)
    if (error != 0) {
        std::cerr << "Could not load glyph" << std::endl;
        return {};
    }

    TextureSettings settings = {};
    settings.dataType = GL_RED;
    Texture texture(settings);
    GL_Call(glActiveTexture(GL_TEXTURE0));
    texture.bind();

    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    FT_GlyphSlot glyph = face->glyph;
    FT_Bitmap bitmap = glyph->bitmap;
    texture.update(reinterpret_cast<const unsigned char *>(bitmap.buffer), bitmap.width, bitmap.rows, 1);

    return std::make_optional<Character>({
          character,
          texture,
          glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
          glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
          face->glyph->advance.x,
          static_cast<long>(characterHeight),
    });
}

void Text::unload() const {
    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

std::optional<Character> Text::character(char c) {
    auto itr = characters.find(c);
    if (itr == characters.end()) {
        return {};
    }
    return std::make_optional(itr->second);
}
