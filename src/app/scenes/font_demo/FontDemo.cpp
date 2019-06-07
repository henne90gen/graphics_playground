#include "FontDemo.h"

void FontDemo::setup() {
    shader = new Shader("../../../src/app/scenes/font_demo/FontDemo.vertex",
                        "../../../src/app/scenes/font_demo/FontDemo.fragment");
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
//    static auto translation = glm::vec3(-0.7F, 0.6F, 0.0F);
    static auto translation = glm::vec3(0.0F);
    static float scale = 0.02F;
    static bool usePixelHeight = true;
    static int characterHeight = 64;
    const float dragSpeed = 0.01;

    ImGui::Begin("Settings");
    ImGui::ColorEdit3("Color", reinterpret_cast<float *>(&color));
    ImGui::DragFloat2("Translation", reinterpret_cast<float *>(&translation), dragSpeed);
    ImGui::DragFloat("Scale", &scale, dragSpeed, 0.01F, 10.0F);
    ImGui::Checkbox("Use Pixel Height", &usePixelHeight);
    ImGui::SliderInt("Character Height", &characterHeight, 1, 100);
    if ((face != nullptr) && (face->glyph != nullptr)) {
        ImGui::Text("Bitmap Pitch: %d", face->glyph->bitmap.pitch);
        ImGui::Text("Bitmap (Top,Left): (%d,%d)", face->glyph->bitmap_left, face->glyph->bitmap_top);
    }
    ImGui::End();

    if (settingsHaveChanged(usePixelHeight, characterHeight)) {
        loadFont(usePixelHeight, characterHeight);
    }

    shader->bind();
    vertexArray->bind();

    shader->setUniform("textColor", color);

    const unsigned int numColumns = 10;
    for (unsigned long i = 0; i < characters.size(); i++) {
        const unsigned int row = i / numColumns;
        const unsigned int column = i % numColumns;
        glm::vec3 offset = glm::vec3(0.05F * column, -0.05F * row, 0.0F);
        renderCharacter(characters[i].texture, translation + offset, scale);
    }

    vertexArray->unbind();
    shader->unbind();
}

void FontDemo::renderCharacter(const Texture &texture, const glm::vec3 &translation, float scale) const {
    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale, scale, scale));
    modelMatrix = glm::translate(modelMatrix, translation / scale);
    shader->setUniform("model", modelMatrix);
    texture.bind();
    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
    texture.unbind();
}

void FontDemo::loadFont(bool usePixelSize, int characterHeight) {
    if (library == nullptr) {
        const int error = FT_Init_FreeType(&library);
        if (error != 0) {
            std::cerr << "Could not initialize freetype" << std::endl;
        }
    }

//    const char *fontName = "../../../src/app/scenes/font_demo/fonts/STANFORD_FREE.ttf";
    const char *fontName = "../../../src/app/scenes/font_demo/fonts/CRACKROCK.ttf";
    long faceIndex = 0;
    int error = FT_New_Face(library, fontName, faceIndex, &face);
    if (error == FT_Err_Unknown_File_Format) {
        std::cerr << "Corrupt font file" << std::endl;
    } else if (error != 0) {
        std::cerr << "Could not read font file" << std::endl;
    }

    {
        std::cout << "Loaded Font" << std::endl;
        std::cout << "\tNumber of faces: " << face->num_faces << std::endl;
        std::cout << "\tFace index: " << face->face_index << std::endl;
        std::cout << "\tFace flags: " << toBits(face->face_flags) << std::endl;
        std::cout << "\tStyle flags: " << toBits(face->style_flags) << std::endl;
        std::cout << "\tNumber of glyphs: " << face->num_glyphs << std::endl;
        std::cout << "\tFamily name: " << face->family_name << std::endl;
        std::cout << "\tStyle name: " << face->style_name << std::endl;
        std::cout << "\tNumber of fixed sizes: " << face->num_fixed_sizes << std::endl;
        std::cout << "\tAvailable sizes: " << face->available_sizes << std::endl;
        std::cout << "\tNumber of character maps: " << face->num_charmaps << std::endl;
        std::cout << "\tCharacter maps: " << face->charmaps << std::endl;
        std::cout << "\tBounding box: (" << face->bbox.xMin << "," << face->bbox.yMin << ") - (" << face->bbox.xMax
                  << ","
                  << face->bbox.yMax << ")" << std::endl;
        std::cout << "\tUnits per EM: " << face->units_per_EM << std::endl;
        std::cout << "\tAscender: " << face->ascender << std::endl;
        std::cout << "\tDescender: " << face->descender << std::endl;
        std::cout << "\tHeight: " << face->height << std::endl;
        std::cout << "\tMaximum advance width: " << face->max_advance_width << std::endl;
        std::cout << "\tMaximum advance height: " << face->max_advance_height << std::endl;
        std::cout << "\tUnderline position: " << face->underline_position << std::endl;
        std::cout << "\tUnderline thickness: " << face->underline_thickness << std::endl;
        std::cout << "\tGlyph: " << face->glyph << std::endl;
        std::cout << "\tSize: " << face->size << std::endl;
        std::cout << "\tCharacter map: " << face->charmap << std::endl;;
    }

    if (!usePixelSize) {
        const int charWidth = 0;
        const int charHeight = characterHeight * 64;
        const int horizontalResolution = 0;
        const int verticalResolution = 0;
        error = FT_Set_Char_Size(face, charWidth, charHeight, horizontalResolution, verticalResolution);
        if (error != 0) {
            std::cerr << "Could not set the character size." << std::endl;
        }
    } else {
        const int pixelWidth = 0;
        error = FT_Set_Pixel_Sizes(face, pixelWidth, characterHeight);
        if (error != 0) {
            std::cerr << "Could not set the character pixel size." << std::endl;
        }
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

    FT_Bitmap bitmap = face->glyph->bitmap;
    texture.update(reinterpret_cast<const char *>(bitmap.buffer), bitmap.width, bitmap.rows, 1);

    return {characterCode, texture};
}

bool settingsHaveChanged(bool height, int characterHeight) {
    static bool initialized = false;
    static bool lastUsePixelHeight = true;
    static int lastCharacterHeight = 0;
    if (!initialized) {
        initialized = true;
        lastUsePixelHeight = height;
        lastCharacterHeight = characterHeight;
        return true;
    }
    bool result = false;
    if (lastUsePixelHeight != height || lastCharacterHeight != characterHeight) {
        result = true;
    }
    lastUsePixelHeight = height;
    lastCharacterHeight = characterHeight;
    return result;
}
