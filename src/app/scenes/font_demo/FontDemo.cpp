#include "FontDemo.h"

#include <filesystem>

void FontDemo::setup() {
    shader = new Shader("../../../src/app/scenes/font_demo/FontDemoVert.glsl",
                        "../../../src/app/scenes/font_demo/FontDemoFrag.glsl");
    shader->bind();

    vertexArray = new VertexArray();
    vertexArray->bind();

    std::vector<float> vertices = {
            -1.0, 1.0, 0.0, 0.0,  //
            -1.0, -1.0, 0.0, 1.0,  //
            1.0, -1.0, 1.0, 1.0, //
            -1.0, 1.0, 0.0, 0.0,  //
            1.0, -1.0, 1.0, 1.0, //
            1.0, 1.0, 1.0, 0.0  //
    };
    auto *buffer = new VertexBuffer(vertices);
    VertexBufferLayout bufferLayout;
    bufferLayout.add<float>(shader, "position", 2);
    bufferLayout.add<float>(shader, "vertexUV", 2);
    vertexArray->addBuffer(*buffer, bufferLayout);

    GL_Call(glEnable(GL_BLEND));
    GL_Call(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    shader->setUniform<int>("textureSampler", 0);
}

std::string toBits(long number) {
    std::string r;
    while (number != 0) {
        r = (number % 2 == 0 ? "0" : "1") + r;
        number /= 2;
    }
    return r;
}

void FontDemo::destroy() {
    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

void FontDemo::tick() {
    std::string text = "Jeb quickly drove a few extra miles on the glazed pavement";
    static auto color = glm::vec3(1.0F, 1.0F, 1.0F);
    static auto translation = glm::vec2(-16, 11);
    static float zoom = 0.02F;
    static unsigned int characterResolution = 64;
    static unsigned int selectedFontIndex = 0;

    std::vector<std::string> fontPaths = getFontPaths();

    showSettings(fontPaths, color, translation, zoom, characterResolution, selectedFontIndex, face);
    if (settingsHaveChanged(characterResolution, selectedFontIndex)) {
        loadFont(fontPaths[selectedFontIndex], characterResolution);
    }

    shader->bind();
    vertexArray->bind();

    shader->setUniform("textColor", color);

    renderAlphabet(translation, zoom);

    const glm::vec2 textTranslation = translation - glm::vec2(5.0F, -10.0F);
    renderText(text, textTranslation, zoom);

    vertexArray->unbind();
    shader->unbind();
}

void FontDemo::renderAlphabet(const glm::vec2 &translation, float zoom) const {
    const unsigned int numColumns = 10;
    for (unsigned long i = 0; i < characters.size(); i++) {
        const unsigned int row = i / numColumns;
        const unsigned int column = i % numColumns;
        glm::vec2 offset = glm::vec2(5.0F * column, -5.0F * row);
        renderCharacter(characters[i], translation + offset, zoom);
    }
}

void FontDemo::renderText(std::string &text, const glm::vec2 &translation, float zoom) {
    for (unsigned long i = 0; i < text.size(); i++) {
        const char c = text[i];
        glm::vec2 offset = glm::vec2(5.0F * i, 0.0F);
        renderCharacter(characters[static_cast<int>(c) - 32], translation + offset, zoom);
    }
}

void FontDemo::renderCharacter(const Character &character, const glm::vec2 &translation, float scale) const {
    if (character.width == 0 || character.height == 0) {
        return;
    }

    float horizontalScale = 1.0F;
    float verticalScale = 1.0F;
    if (character.width < character.height) {
        horizontalScale /= (static_cast<float>(character.height) / static_cast<float>(character.width));
    } else if (character.width > character.height) {
        verticalScale /= (static_cast<float>(character.width) / static_cast<float>(character.height));
    }

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale, scale, scale));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(translation, 0.0F));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(horizontalScale, verticalScale, 1.0F));
    shader->setUniform("model", modelMatrix);

    character.texture.bind();
    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
    character.texture.unbind();
}

void FontDemo::loadFont(std::string &fontPath, int characterHeight) {
    if (library == nullptr) {
        const int error = FT_Init_FreeType(&library);
        if (error != 0) {
            std::cerr << "Could not initialize freetype" << std::endl;
        }
    }

    long faceIndex = 0;
    int error = FT_New_Face(library, fontPath.c_str(), faceIndex, &face);
    if (error == FT_Err_Unknown_File_Format) {
        std::cerr << "Corrupt font file" << std::endl;
    } else if (error != 0) {
        std::cerr << "Could not read font file" << std::endl;
    }

    const int pixelWidth = 0;
    error = FT_Set_Pixel_Sizes(face, pixelWidth, characterHeight);
    if (error != 0) {
        std::cerr << "Could not set the character pixel size." << std::endl;
    }

    loadAlphabet();
}

void FontDemo::loadAlphabet() {
    characters.clear();
    for (unsigned int i = 32; i < 127; i++) {
        Character character = loadCharacter(i);
        characters.push_back(character);
    }
}

Character FontDemo::loadCharacter(unsigned long characterCode) {
    int error = FT_Load_Char(face, characterCode, FT_LOAD_RENDER);
    if (error != 0) {
        std::cerr << "Could not load glyph" << std::endl;
    }

    Texture texture(GL_RED);
    GL_Call(glActiveTexture(GL_TEXTURE0));
    texture.bind();

    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    FT_GlyphSlot glyph = face->glyph;
    FT_Bitmap bitmap = glyph->bitmap;
    texture.update(reinterpret_cast<const char *>(bitmap.buffer), bitmap.width, bitmap.rows, 1);

    return {
            characterCode,
            texture,
            glyph->metrics.width,
            glyph->metrics.height
    };
}

std::vector<std::string> FontDemo::getFontPaths() {
    std::vector<std::string> result = {};
    std::string path = "../../../src/app/scenes/font_demo/fonts";
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
        std::string fileName = entry.path().u8string();;
        result.push_back(fileName);
    }
    return result;
}

void showSettings(const std::vector<std::string> &fontPaths, glm::vec3 &color, glm::vec2 &translation, float &zoom,
                  unsigned int &characterResolution, unsigned int &selectedFontIndex,
                  FT_Face &face) {
    const float translationDragSpeed = 0.1;
    const float scaleDragSpeed = 0.001;

    ImGui::Begin("Settings");
    ImGui::ColorEdit3("Color", reinterpret_cast<float *>(&color));
    ImGui::DragFloat2("Translation", reinterpret_cast<float *>(&translation), translationDragSpeed);
    ImGui::DragFloat("Zoom", &zoom, scaleDragSpeed, 0.001F, 10.0F);
    ImGui::SliderInt("Character Resolution", reinterpret_cast<int *>(&characterResolution), 1, 1000);

    ImGui::ListBoxHeader("Font", fontPaths.size());
    std::string path = "../../../src/app/scenes/font_demo/fonts";
    for (unsigned long i = 0; i < fontPaths.size(); i++) {
        auto fileName = fontPaths[i].substr(path.size() + 1);
        if (ImGui::Selectable(fileName.c_str(), i == selectedFontIndex)) {
            selectedFontIndex = i;
        }
    }
    ImGui::ListBoxFooter();

    if (face != nullptr) {
        showFontInfo(face);
    }
    ImGui::End();
}

void showFontInfo(FT_Face &f) {
    ImGui::Text("Number of faces: %ld", f->num_faces);
    ImGui::Text("Face index: %ld", f->face_index);
    ImGui::Text("Face flags: %s", toBits(f->face_flags).c_str());
    ImGui::Text("Style flags: %s", toBits(f->style_flags).c_str());
    ImGui::Text("Number of glyphs: %ld", f->num_glyphs);
    ImGui::Text("Family name: %s", f->family_name);
    ImGui::Text("Style name: %s", f->style_name);
    ImGui::Text("Number of fixed sizes: %d", f->num_fixed_sizes);
    ImGui::Text("Number of character maps: %d", f->num_charmaps);
    ImGui::Text("Bounding box: (%ld,%ld) - (%ld,%ld)", f->bbox.xMin, f->bbox.yMin, f->bbox.xMax, f->bbox.yMax);
    ImGui::Text("Units per EM: %d", f->units_per_EM);
    ImGui::Text("Ascender: %d", f->ascender);
    ImGui::Text("Descender: %d", f->descender);
    ImGui::Text("Height: %d", f->height);
    ImGui::Text("Maximum advance width: %d", f->max_advance_width);
    ImGui::Text("Maximum advance height: %d", f->max_advance_height);
    ImGui::Text("Underline position: %d", f->underline_position);
    ImGui::Text("Underline thickness: %d", f->underline_thickness);
    ImGui::Text("Size Metrics");
    FT_Size_Metrics &metrics = f->size->metrics;
    ImGui::Text("\tHeight: %ld", metrics.height);
}

bool settingsHaveChanged(int characterHeight, int selectedFontIndex) {
    static bool initialized = false;
    static int lastCharacterHeight = 0;
    static int lastSelectedFontIndex = 0;
    if (!initialized) {
        initialized = true;
        lastCharacterHeight = characterHeight;
        lastSelectedFontIndex = selectedFontIndex;
        return true;
    }
    bool result = false;
    if (lastCharacterHeight != characterHeight ||
        lastSelectedFontIndex != selectedFontIndex) {
        result = true;
    }
    lastCharacterHeight = characterHeight;
    lastSelectedFontIndex = selectedFontIndex;
    return result;
}
