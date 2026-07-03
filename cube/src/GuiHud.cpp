#include "cube/GuiHud.hpp"

#include "cube/TextureCatalog.hpp"
#include "plasma/Engine.hpp"
#include "plasma/UiDraw.hpp"

namespace cube {

namespace {

constexpr const char* kHudIcons[] = {
    "skills.png", "crafting.png", "inventory.png", "worldmap.png", "system.png", "help.png",
};

} // namespace

bool GuiHud::init(TextureCatalog& textures, plasma::Engine& engine) {
    icons_.clear();
    for (const char* key : kHudIcons) {
        IconSlot slot;
        slot.pngKey = key;
        slot.texture = textures.gpuTexture(engine, key);
        if (slot.texture) {
            icons_.push_back(std::move(slot));
        }
    }
    return !icons_.empty();
}

void GuiHud::layout(int screenW, int screenH) {
    const float barH = 72.f;
    const float iconSize = 48.f;
    const float gap = 12.f;
    const float totalW = static_cast<float>(icons_.size()) * iconSize +
                         static_cast<float>(icons_.size() > 1 ? icons_.size() - 1 : 0) * gap;
    const float startX = (static_cast<float>(screenW) - totalW) * 0.5f;
    const float y = static_cast<float>(screenH) - barH + (barH - iconSize) * 0.5f;

    barBounds_ = {0.f, static_cast<float>(screenH) - barH, static_cast<float>(screenW), barH};

    float x = startX;
    for (IconSlot& slot : icons_) {
        slot.bounds = {x, y, iconSize, iconSize};
        x += iconSize + gap;
    }
}

void GuiHud::draw(plasma::Engine& engine, int screenW, int screenH) {
    layout(screenW, screenH);
    plasma::drawSolidQuad(engine, barBounds_, screenW, screenH, {0.08f, 0.09f, 0.12f, 0.92f});
    for (const IconSlot& slot : icons_) {
        plasma::drawTexturedQuad(engine, slot.bounds, screenW, screenH, slot.texture);
    }
}

} // namespace cube
