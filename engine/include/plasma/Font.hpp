#pragma once

#include "plasma/Engine.hpp"
#include "plasma/Math.hpp"
#include "plasma/NamedObject.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace plasma {

class Font;

enum class RetailFontSlot { Ui, Script };

bool initRetailFonts(const std::string& assetRoot);
const std::string& retailFontRoot();
bool retailFontsReady(RetailFontSlot slot = RetailFontSlot::Ui);

Font& uiFont();
Font& scriptFont();

class Font : public NamedObject {
public:
    ~Font() override = default;
    virtual float lineHeight() const = 0;
    virtual float charWidth() const { return lineHeight() * 0.6f; }
    virtual void drawText(Engine& engine, const std::wstring& text, float x, float y, const Vec4& color,
                          int screenW, int screenH) = 0;
};

class PixelFont : public Font {
public:
    explicit PixelFont(Texture* atlas = nullptr);
    float lineHeight() const override;
    float charWidth() const override;
    void drawText(Engine& engine, const std::wstring& text, float x, float y, const Vec4& color,
                  int screenW, int screenH) override;

    void setAtlas(Texture* atlas) { atlas_ = atlas; }
    void setGlyphSize(int w, int h);

private:
    Texture* atlas_{nullptr};
    float lineHeight_{12.f};
    float charWidth_{8.f};
    int glyphW_{8};
    int glyphH_{12};
};

class PlasmaFont : public Font {
public:
    explicit PlasmaFont(RetailFontSlot slot = RetailFontSlot::Ui);
    float lineHeight() const override;
    float charWidth() const override;
    void drawText(Engine& engine, const std::wstring& text, float x, float y, const Vec4& color,
                  int screenW, int screenH) override;

private:
    RetailFontSlot slot_{RetailFontSlot::Ui};
};

class ScalableFont : public Font {
public:
    ScalableFont(float size = 16.f, RetailFontSlot slot = RetailFontSlot::Ui);
    float lineHeight() const override { return size_; }
    void drawText(Engine& engine, const std::wstring& text, float x, float y, const Vec4& color,
                  int screenW, int screenH) override;

private:
    float size_{16.f};
    RetailFontSlot slot_{RetailFontSlot::Ui};
};

class ScriptFont : public Font {
public:
    ScriptFont();
    float lineHeight() const override;
    float charWidth() const override;
    void drawText(Engine& engine, const std::wstring& text, float x, float y, const Vec4& color,
                  int screenW, int screenH) override;

private:
    PlasmaFont font_{RetailFontSlot::Script};
};

class FontEngine : public Object {
public:
    bool init(const std::string& assetRoot);
    Font* load(const std::string& name, const uint8_t* data, size_t size);
    Font* loadPixel(const std::string& name, Texture* atlas, int glyphW = 8, int glyphH = 12);
    Font* get(const std::string& name) const;
    Font* defaultFont();
    Font* scriptFont();

private:
    std::unordered_map<std::string, std::unique_ptr<Font>> fonts_;
};

} // namespace plasma
