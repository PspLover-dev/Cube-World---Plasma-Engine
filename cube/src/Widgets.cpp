#include "cube/Widgets.hpp"

namespace cube {

CubeWidget::CubeWidget(const char* name) {
    setName(name);
}

AdaptionWidget::AdaptionWidget() : CubeWidget("AdaptionWidget") {}
BlueprintPreviewWidget::BlueprintPreviewWidget() : CubeWidget("BlueprintPreviewWidget") {}
CharacterPreviewWidget::CharacterPreviewWidget() : CubeWidget("CharacterPreviewWidget") {}
CharacterStyleWidget::CharacterStyleWidget() : CubeWidget("CharacterStyleWidget") {}
CharacterWidget::CharacterWidget() : CubeWidget("CharacterWidget") {}
ChatWidget::ChatWidget() : CubeWidget("ChatWidget") {}
EnchantWidget::EnchantWidget() : CubeWidget("EnchantWidget") {}
InventoryWidget::InventoryWidget() : CubeWidget("InventoryWidget") {}
MapOverlayWidget::MapOverlayWidget() : CubeWidget("MapOverlayWidget") {}
ObjectiveWidget::ObjectiveWidget() : CubeWidget("ObjectiveWidget") {}
OptionsWidget::OptionsWidget() : CubeWidget("OptionsWidget") {}
PreviewWidget::PreviewWidget() : CubeWidget("PreviewWidget") {}
SkillWidget::SkillWidget() : CubeWidget("SkillWidget") {}
SpeechWidget::SpeechWidget() : CubeWidget("SpeechWidget") {}
SpriteWidget::SpriteWidget() : CubeWidget("SpriteWidget") {}
StartMenuWidget::StartMenuWidget() : CubeWidget("StartMenuWidget") {}
StatisticsWidget::StatisticsWidget() : CubeWidget("StatisticsWidget") {}
SystemWidget::SystemWidget() : CubeWidget("SystemWidget") {}
VoxelWidget::VoxelWidget() : CubeWidget("VoxelWidget") {}
WorldPreviewWidget::WorldPreviewWidget() : CubeWidget("WorldPreviewWidget") {}

void CharacterStyleWidget::update(float dt) {
    plasma::Widget::update(dt);
}

void CharacterStyleWidget::addPartName(std::string name) {
    partNames_.push_back(std::move(name));
}

void ChatWidget::appendLine(std::string line) {
    lines_.push_back(std::move(line));
}

std::unique_ptr<VoxelWidget> VoxelWidget::clone() const {
    return std::make_unique<VoxelWidget>();
}

} // namespace cube
