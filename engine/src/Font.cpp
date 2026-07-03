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
constexpr float kPackPixelSize = 18.f;

struct TtfAtlas {
    std::vector<uint8_t> ttfData;
    Texture* texture{nullptr};
    stbtt_packedchar glyphs[kCharCount]{};
    int texW{0};
    int texH{0};
    float ascent{0.f};
    float lineHeight{16.f};
    float charWidth{9.f};
    bool ready{false};
    bool tried{false};
};

TtfAtlas g_ttfAtlas;

bool loadTtfBytes(std::vector<uint8_t>& out) {
    static const char* const paths[] = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
        "/mingw64/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        nullptr,
    };
    for (const char* const* p = paths; *p; ++p) {
        FILE* f = std::fopen(*p, "rb");
        if (!f) {
            continue;
        }
        std::fseek(f, 0, SEEK_END);
        const long sz = std::ftell(f);
        std::fseek(f, 0, SEEK_SET);
        if (sz <= 0) {
            std::fclose(f);
            continue;
        }
        out.resize(static_cast<size_t>(sz));
        if (std::fread(out.data(), 1, out.size(), f) != out.size()) {
            std::fclose(f);
            out.clear();
            continue;
        }
        std::fclose(f);
        return true;
    }
    return false;
}

bool ensureTtfAtlas(Engine& engine) {
    if (g_ttfAtlas.ready) {
        return true;
    }
    if (g_ttfAtlas.tried) {
        return false;
    }
    g_ttfAtlas.tried = true;

    if (!loadTtfBytes(g_ttfAtlas.ttfData)) {
        return false;
    }

    stbtt_fontinfo info{};
    if (!stbtt_InitFont(&info, g_ttfAtlas.ttfData.data(),
                        stbtt_GetFontOffsetForIndex(g_ttfAtlas.ttfData.data(), 0))) {
        return false;
    }

    int ascent = 0;
    int descent = 0;
    int lineGap = 0;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
    const float scale = stbtt_ScaleForPixelHeight(&info, kPackPixelSize);
    g_ttfAtlas.ascent = scale * static_cast<float>(ascent);
    g_ttfAtlas.lineHeight = scale * static_cast<float>(ascent - descent + lineGap);
    g_ttfAtlas.charWidth = g_ttfAtlas.lineHeight * 0.55f;

    constexpr int kAtlasSize = 512;
    std::vector<uint8_t> alpha(static_cast<size_t>(kAtlasSize) * static_cast<size_t>(kAtlasSize), 0);

    stbtt_pack_context pack{};
    if (!stbtt_PackBegin(&pack, alpha.data(), kAtlasSize, kAtlasSize, 0, 1, nullptr)) {
        return false;
    }
    stbtt_PackSetOversampling(&pack, 2, 2);
    const int packed = stbtt_PackFontRange(&pack, g_ttfAtlas.ttfData.data(), 0, kPackPixelSize, kFirstChar,
                                           kCharCount, g_ttfAtlas.glyphs);
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

    g_ttfAtlas.texW = kAtlasSize;
    g_ttfAtlas.texH = kAtlasSize;
    g_ttfAtlas.texture = engine.createTexture(kAtlasSize, kAtlasSize, rgba.data(), true);
    g_ttfAtlas.ready = g_ttfAtlas.texture != nullptr;
    return g_ttfAtlas.ready;
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

void drawTtfText(Engine& engine, const std::wstring& text, float x, float y, float scale,
                 const Vec4& color, int screenW, int screenH) {
    if (!ensureTtfAtlas(engine) || !g_ttfAtlas.texture) {
        return;
    }

    float penX = 0.f;
    float penBaseline = g_ttfAtlas.ascent;
    float lineY = y;

    for (wchar_t ch : text) {
        if (ch == L'\n') {
            penX = 0.f;
            penBaseline = g_ttfAtlas.ascent;
            lineY += g_ttfAtlas.lineHeight * scale;
            continue;
        }

        const int gi = glyphIndexFor(ch);
        if (gi < 0) {
            continue;
        }

        stbtt_aligned_quad quad{};
        stbtt_GetPackedQuad(g_ttfAtlas.glyphs, g_ttfAtlas.texW, g_ttfAtlas.texH, gi, &penX,
                            &penBaseline, &quad, 1);

        const float gw = (quad.x1 - quad.x0) * scale;
        const float gh = (quad.y1 - quad.y0) * scale;
        if (gw <= 0.f || gh <= 0.f) {
            continue;
        }

        Widget::Rect r{x + quad.x0 * scale, lineY + quad.y0 * scale, gw, gh};
        // OpenGL texture V grows upward; stbtt packed V grows downward.
        drawTexturedQuad(engine, r, screenW, screenH, g_ttfAtlas.texture, color, quad.s0, quad.t1,
                         quad.s1, quad.t0);
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

float PlasmaFont::lineHeight() const {
    return g_ttfAtlas.ready ? g_ttfAtlas.lineHeight : 16.f;
}

float PlasmaFont::charWidth() const {
    return g_ttfAtlas.ready ? g_ttfAtlas.charWidth : 9.f;
}

void PlasmaFont::drawText(Engine& engine, const std::wstring& text, float x, float y, const Vec4& color,
                          int screenW, int screenH) {
    if (ensureTtfAtlas(engine)) {
        drawTtfText(engine, text, x, y, 1.f, color, screenW, screenH);
        return;
    }
    drawSimpleText(engine, text, x, y, charWidth(), lineHeight(), screenW, screenH, color);
}

ScalableFont::ScalableFont(float size) : size_(size) {}

void ScalableFont::drawText(Engine& engine, const std::wstring& text, float x, float y, const Vec4& color,
                            int screenW, int screenH) {
    if (ensureTtfAtlas(engine)) {
        const float scale = size_ / kPackPixelSize;
        drawTtfText(engine, text, x, y, scale, color, screenW, screenH);
        return;
    }
    drawSimpleText(engine, text, x, y, size_ * 0.55f, size_, screenW, screenH, color);
}

Font* FontEngine::load(const std::string& name, const uint8_t*, size_t) {
    auto f = std::make_unique<PlasmaFont>();
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

} // namespace plasma
