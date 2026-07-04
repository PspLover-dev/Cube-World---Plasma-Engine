#pragma once

#include "cube/Creature.hpp"
#include "cube/Database.hpp"
#include "cube/ItemRegistry.hpp"
#include "cube/Region.hpp"
#include "cube/Sprite.hpp"
#include "cube/Terrain.hpp"
#include "cube/WorldInfo.hpp"
#include "cube/Zone.hpp"

#include <memory>
#include <string>
#include <vector>

namespace cube {

class CharacterStyleWidget;

// cube::Spawn — spawn table / entity factory data.
class Spawn {
public:
    Spawn();
    ~Spawn();

    void addCreatureTemplate(uint32_t typeId, std::string name);
    Creature createCreature(uint32_t typeId, uint32_t id) const;

private:
    struct Template {
        uint32_t typeId{0};
        std::string name;
    };
    std::vector<Template> templates_;
    uint32_t nextId_{1};
};

// cube::World — central game world state.
class World {
public:
    World();
    explicit World(const std::string& dataPath, bool loadAssets = true);
    ~World();

    bool load(const std::string& dataPath);
    void update(float dt);

    Database& database() { return database_; }
    SpriteManager& sprites() { return sprites_; }
    Zone& zone() { return zone_; }
    Region& region() { return region_; }
    WorldInfo& worldInfo() { return worldInfo_; }
    const WorldInfo& worldInfo() const { return worldInfo_; }
    ChunkBuffer& chunks() { return chunks_; }
    Spawn& spawn() { return spawn_; }
    ItemRegistry& items() { return itemRegistry_; }
    const ItemRegistry& items() const { return itemRegistry_; }

    Creature* player() { return player_.get(); }
    const Creature* player() const { return player_.get(); }

    void setCharacterStyleWidget(CharacterStyleWidget* widget) { styleWidget_ = widget; }
    CharacterStyleWidget* characterStyleWidget() const { return styleWidget_; }

    uint32_t dayDurationMs() const { return dayDurationMs_; }

private:
    void registerDefaultContent();

    Database database_;
    SpriteManager sprites_;
    Zone zone_;
    Region region_;
    WorldInfo worldInfo_;
    ChunkBuffer chunks_;
    Spawn spawn_;
    ItemRegistry itemRegistry_;
    std::unique_ptr<Creature> player_;
    CharacterStyleWidget* styleWidget_{nullptr};
    uint32_t dayDurationMs_{43200000};
};

} // namespace cube
