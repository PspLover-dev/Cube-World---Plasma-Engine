#include "plasma/Font.hpp"
#include "plasma/UiDraw.hpp"

#include "third_party/stb/stb_truetype.h"

#include <cstdio>
#include <cstring>
#include <vector>

namespace plasma {

namespace {

constexpr int kFirstChar = 32;
constexpr int kCharCount = 224; // Latin-1 printable range 32..255
constexpr float kUiPackPixelSize = 18.f;
constexpr float kScriptPackPixelSize = 22.f;

struct TtfAtlas {
    std::vector<uint8_t> ttfData;
    std::string sourcePath;
    Texture* texture{nullptr};
    stbtt_packedchar glyphs[kCharCount]{};
    int texW{0};
    int texH{0};
    float packPixelSize{kUiPackPixelSize};
    float ascent{0.f};
    float lineHeight{16.f};
    float charWidth{9.f};
    bool ready{false};
    bool tried{false};
};

struct FontLibrary {
    std::string assetRoot;
    TtfAtlas ui;
    TtfAtlas script;
};

FontLibrary g_fonts;

bool loadFontBytesFromPath(std::vector<uint8_t>& out, const std::string& path) {
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        return false;
    }
    std::fseek(f, 0, SEEK_END);
    const long sz = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (sz <= 0) {
        std::fclose(f);
        return false;
    }
    out.resize(static_cast<size_t>(sz));
    if (std::fread(out.data(), 1, out.size(), f) != out.size()) {
        std::fclose(f);
        out.clear();
        return false;
    }
    std::fclose(f);
    return true;
}

TtfAtlas* atlasFor(RetailFontSlot slot) {
    return slot == RetailFontSlot::Script ? &g_fonts.script : &g_fonts.ui;
}

const TtfAtlas& atlasRef(RetailFontSlot slot) {
    return slot == RetailFontSlot::Script ? g_fonts.script : g_fonts.ui;
}

void resetAtlas(TtfAtlas& atlas) {
    atlas.ttfData.clear();
    atlas.texture = nullptr;
    atlas.ready = false;
    atlas.tried = false;
    atlas.ascent = 0.f;
    atlas.lineHeight = atlas.packPixelSize;
    atlas.charWidth = atlas.packPixelSize * 0.55f;
    std::memset(atlas.glyphs, 0, sizeof(atlas.glyphs));
}

bool ensureTtfAtlas(TtfAtlas& atlas, Engine& engine) {
    if (atlas.ready) {
        return true;
    }
    if (atlas.tried) {
        return false;
    }
    atlas.tried = true;

    if (atlas.ttfData.empty() && !atlas.sourcePath.empty()) {
        loadFontBytesFromPath(atlas.ttfData, atlas.sourcePath);
    }
    if (atlas.ttfData.empty()) {
        return false;
    }

    stbtt_fontinfo info{};
    if (!stbtt_InitFont(&info, atlas.ttfData.data(),
                        stbtt_GetFontOffsetForIndex(atlas.ttfData.data(), 0))) {
        return false;
    }

    int ascent = 0;
    int descent = 0;
    int lineGap = 0;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
    const float scale = stbtt_ScaleForPixelHeight(&info, atlas.packPixelSize);
    atlas.ascent = scale * static_cast<float>(ascent);
    atlas.lineHeight = scale * static_cast<float>(ascent - descent + lineGap);
    atlas.charWidth = atlas.lineHeight * 0.55f;

    constexpr int kAtlasSize = 1024;
    std::vector<uint8_t> alpha(static_cast<size_t>(kAtlasSize) * static_cast<size_t>(kAtlasSize), 0);

    stbtt_pack_context pack{};
    if (!stbtt_PackBegin(&pack, alpha.data(), kAtlasSize, kAtlasSize, 0, 1, nullptr)) {
        return false;
    }
    stbtt_PackSetOversampling(&pack, 2, 2);
    const int packed = stbtt_PackFontRange(&pack, atlas.ttfData.data(), 0, atlas.packPixelSize, kFirstChar,
                                           kCharCount, atlas.glyphs);
    stbtt_PackEnd(&pack);
    if (!packed) {
        return false;
    }

    std::vector<uint8_t> rgba(static_cast<size_t>(kAtlasSize) * static_cast<size_t>(kAtlasSize) * 4);
    for (int i = 0; i < kAtlasSize * kAtlasSize; ++i) {
        rgba[static_cast<size_t>(i) * 4 + 0] = 255;
        rgba[static_cast<size_t>(i) * 4 + 1] = 255;
        rgba[static_cast<size_t>(i) * 4 + 2] = 255;
        rgba[static_cast<size_t>(i) * 4 + 3] = alpha[static_cast<size_t>(i)];
    }

    atlas.texW = kAtlasSize;
    atlas.texH = kAtlasSize;
    atlas.texture = engine.createTexture(kAtlasSize, kAtlasSize, rgba.data(), true);
    atlas.ready = atlas.texture != nullptr;
    return atlas.ready;
}

int glyphIndexFor(wchar_t ch) {
    if (ch == L'\n') {
        return -1;
    }
    if (ch < kFirstChar || ch >= kFirstChar + kCharCount) {
        ch = L'?';
    }
    return static_cast<int>(ch) - kFirstChar;
}

void drawTtfText(TtfAtlas& atlas, Engine& engine, const std::wstring& text, float x, float y, float scale,
                 const Vec4& color, int screenW, int screenH) {
    if (!ensureTtfAtlas(atlas, engine) || !atlas.texture) {
        return;
    }

    float penX = 0.f;
    float penBaseline = atlas.ascent;
    float lineY = y;

    for (wchar_t ch : text) {
        if (ch == L'\n') {
            penX = 0.f;
            penBaseline = atlas.ascent;
            lineY += atlas.lineHeight * scale;
            continue;
        }

        const int gi = glyphIndexFor(ch);
        if (gi < 0) {
            continue;
        }

        stbtt_aligned_quad quad{};
        stbtt_GetPackedQuad(atlas.glyphs, atlas.texW, atlas.texH, gi, &penX, &penBaseline, &quad, 1);

        const float gw = (quad.x1 - quad.x0) * scale;
        const float gh = (quad.y1 - quad.y0) * scale;
        if (gw <= 0.f || gh <= 0.f) {
            continue;
        }

        Widget::Rect r{x + quad.x0 * scale, lineY + quad.y0 * scale, gw, gh};
        drawTexturedQuad(engine, r, screenW, screenH, atlas.texture, color, quad.s0, quad.t1, quad.s1,
                         quad.t0);
    }
}

void drawSimpleText(Engine& engine, const std::wstring& text, float x, float y, float charW, float lineH,
                    int screenW, int screenH, const Vec4& color) {
    float cx = x;
    float cy = y;
    for (wchar_t ch : text) {
        if (ch == L'\n') {
            cx = x;
            cy += lineH;
            continue;
        }
        Widget::Rect r{cx, cy, charW, lineH};
        const float shade = 0.4f + static_cast<float>((ch % 16)) / 32.f;
        drawSolidQuad(engine, r, screenW, screenH,
                      {color[0] * shade, color[1] * shade, color[2] * shade, color[3]});
        cx += charW;
    }
}

} // namespace

bool initRetailFonts(const std::string& assetRoot) {
    g_fonts.assetRoot = assetRoot;
    resetAtlas(g_fonts.ui);
    resetAtlas(g_fonts.script);

    g_fonts.ui.sourcePath = assetRoot + "/resource1.dat";
    g_fonts.ui.packPixelSize = kUiPackPixelSize;
    g_fonts.script.sourcePath = assetRoot + "/resource2.dat";
    g_fonts.script.packPixelSize = kScriptPackPixelSize;

    const bool uiLoaded = loadFontBytesFromPath(g_fonts.ui.ttfData, g_fonts.ui.sourcePath);
    const bool scriptLoaded = loadFontBytesFromPath(g_fonts.script.ttfData, g_fonts.script.sourcePath);
    return uiLoaded && scriptLoaded;
}

const std::string& retailFontRoot() {
    return g_fonts.assetRoot;
}

bool retailFontsReady(RetailFontSlot slot) {
    return !atlasRef(slot).ttfData.empty();
}

Font& uiFont() {
    static PlasmaFont font(RetailFontSlot::Ui);
    return font;
}

Font& scriptFont() {
    static ScriptFont font;
    return font;
}

PixelFont::PixelFont(Texture* atlas) : atlas_(atlas) {}

float PixelFont::lineHeight() const {
    return lineHeight_;
}

float PixelFont::charWidth() const {
    return charWidth_;
}

void PixelFont::setGlyphSize(int w, int h) {
    glyphW_ = w;
    glyphH_ = h;
    charWidth_ = static_cast<float>(w);
    lineHeight_ = static_cast<float>(h);
}

void PixelFont::drawText(Engine& engine, const std::wstring& text, float x, float y, const Vec4& color,
                         int screenW, int screenH) {
    if (atlas_) {
        float cx = x;
        for (wchar_t ch : text) {
            if (ch == L'\n') {
                cx = x;
                y += lineHeight_;
                continue;
            }
            const int col = static_cast<int>(ch) % 16;
            const int row = static_cast<int>(ch) / 16;
            Widget::Rect r{cx, y, charWidth_, lineHeight_};
            const float u0 = static_cast<float>(col * glyphW_) / static_cast<float>(atlas_->width());
            const float v0 = static_cast<float>(row * glyphH_) / static_cast<float>(atlas_->height());
            const float u1 = static_cast<float>((col + 1) * glyphW_) / static_cast<float>(atlas_->width());
            const float v1 = static_cast<float>((row + 1) * glyphH_) / static_cast<float>(atlas_->height());
            drawTexturedQuad(engine, r, screenW, screenH, atlas_, color, u0, v0, u1, v1);
            cx += charWidth_;
        }
        return;
    }
    drawSimpleText(engine, text, x, y, charWidth_, lineHeight_, screenW, screenH, color);
}

PlasmaFont::PlasmaFont(RetailFontSlot slot) : slot_(slot) {}

float PlasmaFont::lineHeight() const {
    const TtfAtlas& atlas = atlasRef(slot_);
    return !atlas.ttfData.empty() ? atlas.lineHeight : 16.f;
}

float PlasmaFont::charWidth() const {
    const TtfAtlas& atlas = atlasRef(slot_);
    return !atlas.ttfData.empty() ? atlas.charWidth : 9.f;
}

void PlasmaFont::drawText(Engine& engine, const std::wstring& text, float x, float y, const Vec4& color,
                          int screenW, int screenH) {
    TtfAtlas* atlas = atlasFor(slot_);
    if (atlas && !atlas->ttfData.empty()) {
        drawTtfText(*atlas, engine, text, x, y, 1.f, color, screenW, screenH);
        return;
    }
    drawSimpleText(engine, text, x, y, charWidth(), lineHeight(), screenW, screenH, color);
}

ScalableFont::ScalableFont(float size, RetailFontSlot slot) : size_(size), slot_(slot) {}

void ScalableFont::drawText(Engine& engine, const std::wstring& text, float x, float y, const Vec4& color,
                            int screenW, int screenH) {
    TtfAtlas* atlas = atlasFor(slot_);
    if (atlas && !atlas->ttfData.empty()) {
        const float basePack = atlas->packPixelSize > 0.f ? atlas->packPixelSize : kUiPackPixelSize;
        const float scale = size_ / basePack;
        drawTtfText(*atlas, engine, text, x, y, scale, color, screenW, screenH);
        return;
    }
    drawSimpleText(engine, text, x, y, size_ * 0.55f, size_, screenW, screenH, color);
}

ScriptFont::ScriptFont() = default;

float ScriptFont::lineHeight() const {
    return font_.lineHeight();
}

float ScriptFont::charWidth() const {
    return font_.charWidth();
}

void ScriptFont::drawText(Engine& engine, const std::wstring& text, float x, float y, const Vec4& color,
                          int screenW, int screenH) {
    font_.drawText(engine, text, x, y, color, screenW, screenH);
}

bool FontEngine::init(const std::string& assetRoot) {
    return initRetailFonts(assetRoot);
}

Font* FontEngine::load(const std::string& name, const uint8_t*, size_t) {
    auto f = std::make_unique<PlasmaFont>(RetailFontSlot::Ui);
    f->setName(name);
    Font* raw = f.get();
    fonts_[name] = std::move(f);
    return raw;
}

Font* FontEngine::loadPixel(const std::string& name, Texture* atlas, int glyphW, int glyphH) {
    auto f = std::make_unique<PixelFont>(atlas);
    f->setName(name);
    f->setGlyphSize(glyphW, glyphH);
    Font* raw = f.get();
    fonts_[name] = std::move(f);
    return raw;
}

Font* FontEngine::get(const std::string& name) const {
    const auto it = fonts_.find(name);
    return it != fonts_.end() ? it->second.get() : nullptr;
}

Font* FontEngine::defaultFont() {
    Font* f = get("default");
    if (f) {
        return f;
    }
    return load("default", nullptr, 0);
}

Font* FontEngine::scriptFont() {
    Font* f = get("script");
    if (f) {
        return f;
    }
    auto font = std::make_unique<ScriptFont>();
    font->setName("script");
    Font* raw = font.get();
    fonts_["script"] = std::move(font);
    return raw;
}

} // namespace plasma
