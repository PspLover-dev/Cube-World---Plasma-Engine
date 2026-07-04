#include "cube/World.hpp"

#include "cube/Constants.hpp"

namespace cube {

Spawn::Spawn() = default;
Spawn::~Spawn() = default;

void Spawn::addCreatureTemplate(uint32_t typeId, std::string name) {
    templates_.push_back({typeId, std::move(name)});
}

Creature Spawn::createCreature(uint32_t typeId, uint32_t id) const {
    Creature c(id, typeId);
    for (const auto& t : templates_) {
        if (t.typeId == typeId) {
            c.setName(t.name);
            return c;
        }
    }
    return c;
}

World::World() {
    registerDefaultContent();
    player_ = std::make_unique<Creature>(1, 0x53);
    if (const ItemRegistry::Entry* nomad = itemRegistry_.findItem(0x53)) {
        player_->setName(ItemRegistry::toUtf8(nomad->name));
    } else {
        player_->setName("Player");
    }
}

World::World(const std::string& dataPath, bool loadAssets) : World() {
    if (loadAssets) {
        load(dataPath);
    }
}

World::~World() = default;

void World::registerDefaultContent() {
    region_.setId(0);
    region_.setName("Main");
    worldInfo_.setName("World");
    worldInfo_.setSeed(0);

    for (const auto& item : itemRegistry_.items()) {
        if (item.named) {
            spawn_.addCreatureTemplate(item.id, ItemRegistry::toUtf8(item.name));
        }
    }
}

bool World::load(const std::string& dataPath) {
    if (!database_.open(dataPath)) {
        return false;
    }

    sprites_.add(1, "player_icon");
    zone_.fieldAt(0, 0).resize(64, 64, 256);
    region_.setChunkOccupied(0, 0, true);
    return true;
}

void World::update(float dt) {
    (void)dt;
    worldInfo_.update(this);
}

} // namespace cube
