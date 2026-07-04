#include "cube/ZoneTile.hpp"

namespace cube {

ZoneTileEntry::ZoneTileEntry() {
    listHead = new ZoneTileListNode();
    auto* node = static_cast<ZoneTileListNode*>(listHead);
    node->next = node;
    node->prev = node;
}

ZoneTileEntry::~ZoneTileEntry() {
    delete static_cast<ZoneTileListNode*>(listHead);
    listHead = nullptr;
}

ZoneTile::ZoneTile() {
    entityListHead_ = std::make_unique<ZoneTileListNode>();
    entityList_.next = entityListHead_.get();
    entityList_.prev = entityListHead_.get();
    entityListHead_->next = &entityList_;
    entityListHead_->prev = &entityList_;
}

ZoneTile::~ZoneTile() = default;

} // namespace cube
