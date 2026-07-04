#include "cube/ItemRegistry.hpp"

#include <utility>

namespace cube {

namespace {

std::wstring toWide(std::string s) {
    return std::wstring(s.begin(), s.end());
}

uint64_t equipKey(uint32_t slotId, uint32_t subId) {
    return (static_cast<uint64_t>(slotId) << 32) | subId;
}

} // namespace

ItemRegistry::ItemRegistry() {
    registerRetailCatalog();
}

ItemRegistry::Entry* ItemRegistry::lookupOrCreate(std::vector<Entry>& entries,
                                                  std::unordered_map<uint32_t, size_t>& index,
                                                  uint32_t id) {
    if (auto it = index.find(id); it != index.end()) {
        return &entries[it->second];
    }
    index[id] = entries.size();
    entries.push_back({id, {}, false});
    return &entries.back();
}

ItemRegistry::Entry* ItemRegistry::lookupOrCreateRace(uint32_t id) {
    return lookupOrCreate(races_, raceIndex_, id);
}

ItemRegistry::EquipEntry* ItemRegistry::lookupOrCreateEquip(uint32_t slotId, uint32_t subId) {
    const uint64_t key = equipKey(slotId, subId);
    if (auto it = equipIndex_.find(key); it != equipIndex_.end()) {
        return &equipSlots_[it->second];
    }
    equipIndex_[key] = equipSlots_.size();
    equipSlots_.push_back({slotId, subId, {}, false});
    return &equipSlots_.back();
}

ItemRegistry::Entry* ItemRegistry::lookupOrCreateStatic(uint32_t id) {
    return lookupOrCreate(staticObjects_, staticIndex_, id);
}

ItemRegistry::Entry* ItemRegistry::lookupOrCreateSkill(uint32_t id) {
    return lookupOrCreate(skills_, skillIndex_, id);
}

ItemRegistry::Entry* ItemRegistry::lookupOrCreateAbility(uint32_t id) {
    return lookupOrCreate(abilities_, abilityIndex_, id);
}

void ItemRegistry::assignName(Entry* entry, std::wstring name) {
    if (entry == nullptr) {
        return;
    }
    entry->name = std::move(name);
    entry->named = true;
}

void ItemRegistry::assignName(EquipEntry* entry, std::wstring name) {
    if (entry == nullptr) {
        return;
    }
    entry->name = std::move(name);
    entry->named = true;
}

void ItemRegistry::registerRetailCatalog() {
#include "WorldRegistryInit.inc"
}

void ItemRegistry::registerItem(uint32_t id, std::string name) {
    assignName(lookupOrCreateRace(id), toWide(std::move(name)));
}

void ItemRegistry::registerEquipSlot(uint32_t slotId, uint32_t subId, std::string name) {
    assignName(lookupOrCreateEquip(slotId, subId), toWide(std::move(name)));
}

const ItemRegistry::Entry* ItemRegistry::findRace(uint32_t id) const {
    auto it = raceIndex_.find(id);
    return it != raceIndex_.end() ? &races_[it->second] : nullptr;
}

const ItemRegistry::EquipEntry* ItemRegistry::findEquip(uint32_t slotId, uint32_t subId) const {
    auto it = equipIndex_.find(equipKey(slotId, subId));
    return it != equipIndex_.end() ? &equipSlots_[it->second] : nullptr;
}

std::string ItemRegistry::toUtf8(const std::wstring& wide) {
    return std::string(wide.begin(), wide.end());
}

} // namespace cube
