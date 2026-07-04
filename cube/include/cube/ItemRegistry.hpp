#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace cube {

// Retail name tables: FUN_004e2df0 / FUN_005943b0 lookup + FUN_0040f7a0 / FUN_00468000 assign.
class ItemRegistry {
public:
    struct Entry {
        uint32_t id{0};
        std::wstring name;
        bool named{false};
    };

    struct EquipEntry {
        uint32_t slotId{0};
        uint32_t subId{0};
        std::wstring name;
        bool named{false};
    };

    ItemRegistry();

    // FUN_004e2df0 — race / creature / object type registry (this_01).
    Entry* lookupOrCreateRace(uint32_t id);
    // FUN_005943b0 — equipment category registry (local_48).
    EquipEntry* lookupOrCreateEquip(uint32_t slotId, uint32_t subId);
    // Additional registries in World::World @ 0058eb00.
    Entry* lookupOrCreateStatic(uint32_t id);
    Entry* lookupOrCreateSkill(uint32_t id);
    Entry* lookupOrCreateAbility(uint32_t id);

    // FUN_0040f7a0 / FUN_00468000 — assign wide display name to registry slot.
    void assignName(Entry* entry, std::wstring name);
    void assignName(EquipEntry* entry, std::wstring name);

    void registerRetailCatalog();

    const Entry* findRace(uint32_t id) const;
    const EquipEntry* findEquip(uint32_t slotId, uint32_t subId) const;

    const std::vector<Entry>& races() const { return races_; }
    const std::vector<EquipEntry>& equipSlots() const { return equipSlots_; }
    const std::vector<Entry>& staticObjects() const { return staticObjects_; }
    const std::vector<Entry>& skills() const { return skills_; }
    const std::vector<Entry>& abilities() const { return abilities_; }

    // Legacy convenience used elsewhere in the port.
    void registerItem(uint32_t id, std::string name);
    void registerEquipSlot(uint32_t slotId, uint32_t subId, std::string name);
    const Entry* findItem(uint32_t id) const { return findRace(id); }
    const std::vector<Entry>& items() const { return races_; }

    static std::string toUtf8(const std::wstring& wide);

private:
    Entry* lookupOrCreate(std::vector<Entry>& entries, std::unordered_map<uint32_t, size_t>& index,
                          uint32_t id);

    std::vector<Entry> races_;
    std::unordered_map<uint32_t, size_t> raceIndex_;
    std::vector<EquipEntry> equipSlots_;
    std::unordered_map<uint64_t, size_t> equipIndex_;
    std::vector<Entry> staticObjects_;
    std::unordered_map<uint32_t, size_t> staticIndex_;
    std::vector<Entry> skills_;
    std::unordered_map<uint32_t, size_t> skillIndex_;
    std::vector<Entry> abilities_;
    std::unordered_map<uint32_t, size_t> abilityIndex_;
};

} // namespace cube
