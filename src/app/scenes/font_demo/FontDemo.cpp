#include "FontDemo.h"

#include "util/FileUtils.h"
#include "util/ImGuiUtils.h"

void FontDemo::setup() {
    shader = std::make_shared<Shader>("../../../src/app/scenes/font_demo/FontDemoVert.glsl",
                                      "../../../src/app/scenes/font_demo/FontDemoFrag.glsl");
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();

    std::vector<float> vertices = {
            -0.5, 0.5, 0.0, 0.0,  //
            -0.5, -0.5, 0.0, 1.0,  //
            0.5, -0.5, 1.0, 1.0, //
            -0.5, 0.5, 0.0, 0.0,  //
            0.5, -0.5, 1.0, 1.0, //
            0.5, 0.5, 1.0, 0.0  //
    };
    BufferLayout bufferLayout = {
            {ShaderDataType::Float2, "position"},
            {ShaderDataType::Float2, "vertexUV"}
    };
    auto buffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    vertexArray->addVertexBuffer(buffer);

    shader->setUniform<int>("textureSampler", 0);
}

void FontDemo::destroy() {
    GL_Call(glEnable(GL_DEPTH_TEST));

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

void FontDemo::tick() {
    std::string text = "Jeb quickly drove a few extra miles on the glazed pavement";
    static auto color = glm::vec3(1.0F, 1.0F, 1.0F);
    static auto translation = glm::vec2(-0.16, 0.11);
    static float zoom = 0.112F;
    static unsigned int characterResolution = 512;
    static unsigned int selectedFontIndex = 3;
    static bool shouldRenderAlphabet = true;

    std::vector<std::string> fontPaths = {};

    showSettings(fontPaths, color, translation, zoom, characterResolution, selectedFontIndex, shouldRenderAlphabet,
                 face);
    if (settingsHaveChanged(characterResolution, selectedFontIndex)) {
        loadFont(fontPaths[selectedFontIndex], characterResolution);
    }

    shader->bind();
    vertexArray->bind();

    shader->setUniform("textColor", color);

    if (shouldRenderAlphabet) {
        renderAlphabet(translation, zoom);
    }

    const glm::vec2 textTranslation = translation + glm::vec2(0.1F, 0.1F);
    renderText(text, textTranslation, zoom);

    vertexArray->unbind();
    shader->unbind();
}

void FontDemo::renderAlphabet(const glm::vec2 &translation, float zoom) const {
    setViewMatrix(translation, zoom);

    const unsigned int numColumns = 10;
    for (unsigned long i = 0; i < characters.size(); i++) {
        const unsigned int row = i / numColumns;
        const unsigned int column = i % numColumns;
        glm::vec2 offset = glm::vec2(1.0F * column, -1.0F * row);
        renderCharacter(characters[i], offset);
    }
}

void FontDemo::setViewMatrix(const glm::vec2 &translation, float zoom) const {
    glm::mat4 viewMatrix = glm::mat4(1.0F);
    viewMatrix = glm::translate(viewMatrix, glm::vec3(translation, 0.0F));
    viewMatrix = glm::scale(viewMatrix, glm::vec3(zoom, zoom, zoom));
    shader->setUniform("view", viewMatrix);
}

void FontDemo::renderText(std::string &text, const glm::vec2 &translation, float zoom) {
    setViewMatrix(translation, zoom);

    glm::vec2 nextCharacterPosition = glm::vec2();
    for (char c : text) {
        Character &character = characters[static_cast<int>(c) - 32];
        renderCharacter(character, nextCharacterPosition);
        auto scaledAdvance =
                static_cast<float>(character.advance) / 64.0F / static_cast<float>(character.maxHeight);
        nextCharacterPosition += glm::vec2(scaledAdvance, 0.0F);
    }
}

void FontDemo::renderCharacter(const Character &character, const glm::vec2 &translation) const {
    if (character.dimension.x == 0 || character.dimension.y == 0) {
        return;
    }

    auto offset = glm::vec2();
    offset.x = static_cast<float>(character.offset.x);
    offset.x -= (static_cast<float>(character.maxHeight) - static_cast<float>(character.dimension.x)) / 2.0F;
    offset.y = static_cast<float>(character.dimension.y) - static_cast<float>(character.offset.y);
    offset.y += (static_cast<float>(character.maxHeight) - static_cast<float>(character.dimension.y)) / 2.0F;
    offset.y *= -1.0F;
    offset /= character.maxHeight;
    glm::vec2 finalTranslation = translation + offset;

    float horizontalScale =
            static_cast<float>(character.dimension.x) / static_cast<float>(character.maxHeight);
    float verticalScale =
            static_cast<float>(character.dimension.y) / static_cast<float>(character.maxHeight);

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(finalTranslation, 0.0F));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(horizontalScale, verticalScale, 1.0F));
    shader->setUniform("model", modelMatrix);

    character.texture.bind();

    GL_Call(glEnable(GL_BLEND));
    GL_Call(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL_Call(glDisable(GL_DEPTH_TEST));

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

    loadAlphabet(characterHeight);
}

void FontDemo::loadAlphabet(int characterHeight) {
    characters.clear();
    for (unsigned int i = 32; i < 127; i++) {
        Character character = loadCharacter(i, characterHeight);
        characters.push_back(character);
    }
}

Character FontDemo::loadCharacter(const char character, const int characterHeight) {
    int error = FT_Load_Char(face, character, FT_LOAD_RENDER);
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
    texture.update(reinterpret_cast<const unsigned char *>(bitmap.buffer), bitmap.width, bitmap.rows, 1);

    return {
            character,
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x,
            characterHeight
    };
}

void showSettings(std::vector<std::string> &fontPaths, glm::vec3 &color, glm::vec2 &translation, float &zoom,
                  unsigned int &characterResolution, unsigned int &selectedFontIndex, bool &shouldRenderAlphabet,
                  FT_Face &face) {
    const float translationDragSpeed = 0.01;
    const float scaleDragSpeed = 0.001;

    ImGui::Begin("Settings");
    ImGui::ColorEdit3("Color", reinterpret_cast<float *>(&color));
    ImGui::DragFloat2("Translation", reinterpret_cast<float *>(&translation), translationDragSpeed);
    ImGui::DragFloat("Zoom", &zoom, scaleDragSpeed, 0.001F, 10.0F);
    ImGui::SliderInt("Character Resolution", reinterpret_cast<int *>(&characterResolution), 1, 1000);
    ImGui::Checkbox("Show Alphabet", &shouldRenderAlphabet);

    std::string prefix = "../../../src/app/scenes/font_demo/fonts/";
    ImGui::FileSelector("Font", prefix, selectedFontIndex, fontPaths);

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

std::string toBits(long number) {
    std::string r;
    while (number != 0) {
        r = (number % 2 == 0 ? "0" : "1") + r;
        number /= 2;
    }
    return r;
}
