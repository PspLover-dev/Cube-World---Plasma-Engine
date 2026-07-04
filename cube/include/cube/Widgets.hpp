#pragma once

#include "plasma/Widget.hpp"

#include <memory>
#include <string>
#include <vector>

namespace cube {

class CharacterStyleWidget;

// Base for cube UI widgets (plasma::NamedObject::Widget port).
class CubeWidget : public plasma::Widget {
public:
    explicit CubeWidget(const char* name);
    ~CubeWidget() override = default;
};

class AdaptionWidget : public CubeWidget {
public:
    AdaptionWidget();
};

class BlueprintPreviewWidget : public CubeWidget {
public:
    BlueprintPreviewWidget();
};

class CharacterPreviewWidget : public CubeWidget {
public:
    CharacterPreviewWidget();
};

class CharacterStyleWidget : public CubeWidget {
public:
    CharacterStyleWidget();
    void update(float dt) override;

    void setEngine(plasma::Engine* engine) { engine_ = engine; }
    plasma::Engine* engine() const { return engine_; }

    uint32_t tintColor() const { return tintColor_; } // offset_0x40 = 0xff32c8ff

    void addPartName(std::string name);
    const std::vector<std::string>& partNames() const { return partNames_; }

private:
    plasma::Engine* engine_{nullptr};
    uint32_t tintColor_{0xff32c8ff};
    uint16_t styleFlags_{0xffff}; // offset_0x44
    std::vector<std::string> partNames_;
};

class CharacterWidget : public CubeWidget {
public:
    CharacterWidget();
};

class ChatWidget : public CubeWidget {
public:
    ChatWidget();
    void appendLine(std::string line);
    const std::vector<std::string>& lines() const { return lines_; }

private:
    std::vector<std::string> lines_;
};

class EnchantWidget : public CubeWidget {
public:
    EnchantWidget();
};

class InventoryWidget : public CubeWidget {
public:
    InventoryWidget();
    void setChatWidget(ChatWidget* chat) { chat_ = chat; }

private:
    ChatWidget* chat_{nullptr};
};

class MapOverlayWidget : public CubeWidget {
public:
    MapOverlayWidget();
};

class ObjectiveWidget : public CubeWidget {
public:
    ObjectiveWidget();
};

class OptionsWidget : public CubeWidget {
public:
    OptionsWidget();
};

class PreviewWidget : public CubeWidget {
public:
    PreviewWidget();
};

class SkillWidget : public CubeWidget {
public:
    SkillWidget();
};

class SpeechWidget : public CubeWidget {
public:
    SpeechWidget();
};

class SpriteWidget : public CubeWidget {
public:
    SpriteWidget();
};

class StartMenuWidget : public CubeWidget {
public:
    StartMenuWidget();
};

class StatisticsWidget : public CubeWidget {
public:
    StatisticsWidget();
};

class SystemWidget : public CubeWidget {
public:
    SystemWidget();
};

class VoxelWidget : public CubeWidget {
public:
    VoxelWidget();
    std::unique_ptr<VoxelWidget> clone() const;
};

class WorldPreviewWidget : public CubeWidget {
public:
    WorldPreviewWidget();
    void setCharacterStyleWidget(CharacterStyleWidget* style) { style_ = style; }

private:
    CharacterStyleWidget* style_{nullptr};
};

} // namespace cube
