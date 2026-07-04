#pragma once

#include <cstdint>
#include <memory>

namespace cube {

struct ZoneTileListNode {
    ZoneTileListNode* next{nullptr};
    ZoneTileListNode* prev{nullptr};
};

class ZoneTile {
public:
    ZoneTile();
    ~ZoneTile();

    ZoneTile(const ZoneTile&) = delete;
    ZoneTile& operator=(const ZoneTile&) = delete;

    int refCount() const { return refCount_; }
    ZoneTileListNode& entityList() { return entityList_; }

private:
    int field0_{0};
    void* field4_{nullptr};
    int field8_{0};
    int fieldC_{0};
    int field10_{0};
    int refCount_{1};
    int field18_{0};
    std::unique_ptr<ZoneTileListNode> entityListHead_;
    ZoneTileListNode entityList_{};
    int field24_{0};
    int field28_{0};
    int field2C_{0};
};

struct ZoneTileEntry {
    void* listHead{nullptr};
    uint32_t reserved{0};

    ZoneTileEntry();
    ~ZoneTileEntry();
};

} // namespace cube
