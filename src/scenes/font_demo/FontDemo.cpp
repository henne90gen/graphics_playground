#include "FontDemo.h"

#include "Main.h"
#include "util/FileUtils.h"
#include "util/ImGuiUtils.h"

DEFINE_SCENE_MAIN(FontDemo)
DEFINE_DEFAULT_SHADERS(font_demo_FontDemo)

void FontDemo::setup() {
    shader = CREATE_DEFAULT_SHADER(font_demo_FontDemo);
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();

    std::vector<float> vertices = {
          -0.5, 0.5,  0.0, 0.0, // NOLINT
          -0.5, -0.5, 0.0, 1.0, // NOLINT
          0.5,  -0.5, 1.0, 1.0, // NOLINT
          -0.5, 0.5,  0.0, 0.0, // NOLINT
          0.5,  -0.5, 1.0, 1.0, // NOLINT
          0.5,  0.5,  1.0, 0.0  // NOLINT
    };
    BufferLayout bufferLayout = {{ShaderDataType::Float2, "position"}, {ShaderDataType::Float2, "vertexUV"}};
    auto buffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    vertexArray->addVertexBuffer(buffer);

    shader->setUniform("textureSampler", 0);

    GL_Call(glEnable(GL_BLEND));
    GL_Call(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL_Call(glDisable(GL_DEPTH_TEST));
}

void FontDemo::destroy() { GL_Call(glEnable(GL_DEPTH_TEST)); }

void FontDemo::tick() {
    std::string text = "Jeb quickly drove a few extra miles on the glazed pavement";
    static auto color = glm::vec3(1.0F, 1.0F, 1.0F);  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto translation = glm::vec2(-0.16, 0.11); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static float zoom = 0.112F;                       // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static unsigned int characterResolution = 512;    // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static unsigned int selectedFontIndex = 3;        // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static bool shouldRenderAlphabet = true;

    std::vector<std::string> fontPaths = {};

    showSettings(fontPaths, color, translation, zoom, characterResolution, selectedFontIndex, shouldRenderAlphabet, t);
    if (settingsHaveChanged(characterResolution, selectedFontIndex) && !fontPaths.empty()) {
        t.load(fontPaths[selectedFontIndex], characterResolution);
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

void FontDemo::renderAlphabet(const glm::vec2 &translation, float zoom) {
    setViewMatrix(translation, zoom);

    const unsigned int numColumns = 10;
    const unsigned int firstCharacter = 32;
    const unsigned int lastCharacter = 127;
    for (unsigned int i = firstCharacter; i < lastCharacter; i++) {
        char c = static_cast<char>(i);
        std::optional<Character> characterOpt = t.character(c);
        if (!characterOpt.has_value()) {
            continue;
        }
        const unsigned int row = i / numColumns;
        const unsigned int column = i % numColumns;
        glm::vec2 offset = glm::vec2(1.0F * static_cast<float>(column), -1.0F * static_cast<float>(row));
        renderCharacter(characterOpt.value(), offset);
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
        std::optional<Character> characterOpt = t.character(c);
        if (!characterOpt.has_value()) {
            continue;
        }
        auto &character = characterOpt.value();
        renderCharacter(character, nextCharacterPosition);
        const float characterToPixelSpace = 64.0F;
        auto scaledAdvance =
              static_cast<float>(character.advance) / characterToPixelSpace / static_cast<float>(character.maxHeight);
        nextCharacterPosition += glm::vec2(scaledAdvance, 0.0F);
    }
}

void FontDemo::renderCharacter(const Character &character, const glm::vec2 &translation) const {
    if (character.dimension.x == 0 || character.dimension.y == 0) {
        return;
    }

    const float scale = 2.0F;
    auto offset = character.offset(scale);
    glm::vec2 finalTranslation = translation + offset;
    auto characterScale = character.scale();

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(finalTranslation, 0.0F));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(characterScale, 1.0F));
    shader->setUniform("model", modelMatrix);

    character.texture.bind();

    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
    Texture::unbind();
}

void showSettings(std::vector<std::string> &fontPaths, glm::vec3 &color, glm::vec2 &translation, float &zoom,
                  unsigned int &characterResolution, unsigned int &selectedFontIndex, bool &shouldRenderAlphabet,
                  Text &t) {
    const float translationDragSpeed = 0.01;
    const float scaleDragSpeed = 0.001;
    const float minZoom = 0.001F;
    const float maxZoom = 10.0F;
    const int minResolution = 1;
    const int maxResolution = 1000;

    ImGui::Begin("Settings");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::ColorEdit3("Color", reinterpret_cast<float *>(&color));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat2("Translation", reinterpret_cast<float *>(&translation), translationDragSpeed);
    ImGui::DragFloat("Zoom", &zoom, scaleDragSpeed, minZoom, maxZoom);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::SliderInt("Character Resolution", reinterpret_cast<int *>(&characterResolution), minResolution,
                     maxResolution);
    ImGui::Checkbox("Show Alphabet", &shouldRenderAlphabet);

    std::string prefix = "font_demo_resources/fonts/";
    ImGui::FileSelector("Font", prefix, selectedFontIndex, fontPaths);

    showFontInfo(t.face);

    ImGui::End();
}

void showFontInfo(FT_Face &f) {
    if (f == nullptr) {
        return;
    }
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

bool settingsHaveChanged(unsigned int characterHeight, unsigned int selectedFontIndex) {
    static bool initialized = false;
    static unsigned int lastCharacterHeight = 0;
    static unsigned int lastSelectedFontIndex = 0;
    if (!initialized) {
        initialized = true;
        lastCharacterHeight = characterHeight;
        lastSelectedFontIndex = selectedFontIndex;
        return true;
    }
    bool result = false;
    if (lastCharacterHeight != characterHeight || lastSelectedFontIndex != selectedFontIndex) {
        result = true;
    }
    lastCharacterHeight = characterHeight;
    lastSelectedFontIndex = selectedFontIndex;
    return result;
}

std::string toBits(long number) {
    std::string r;
    while (number != 0) {
        // NOLINTNEXTLINE(performance-inefficient-string-concatenation,cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
        r = (number % 2 == 0 ? "0" : "1") + r;
        number /= 2;
    }
    return r;
}
