#pragma once

#include "plasma/Widget.hpp"

#include <string>
#include <vector>

namespace plasma {
class Engine;
class Texture;
} // namespace plasma

namespace cube {

class TextureCatalog;

/// Bottom HUD icon bar (GameController retail mapping from data3.db).
class GuiHud {
public:
    struct IconSlot {
        std::string pngKey;
        plasma::Widget::Rect bounds{};
        plasma::Texture* texture{nullptr};
    };

    bool init(TextureCatalog& textures, plasma::Engine& engine);
    void layout(int screenW, int screenH);
    void draw(plasma::Engine& engine, int screenW, int screenH);

    const std::vector<IconSlot>& icons() const { return icons_; }

private:
    std::vector<IconSlot> icons_;
    plasma::Widget::Rect barBounds_{};
};

} // namespace cube
