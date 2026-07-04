#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace cube {

class ModelCatalog;

struct RegistryNamedId {
    uint32_t id{0};
    uint32_t subId{0};
    std::string name;
};

struct RegistryCreatureModel {
    std::string prefix;
    bool multiPart{false};
};

struct RegistryEquipModel {
    std::string modelKey;
};

class RegistryModelResolver {
public:
    void rebuild(const ModelCatalog& catalog);

    std::optional<RegistryCreatureModel> resolveCreature(const std::string& registryName) const;
    std::optional<RegistryEquipModel> resolveEquip(const std::string& registryName) const;

    static std::vector<RegistryNamedId> worldRaces();
    static std::vector<RegistryNamedId> worldItems();
    static std::vector<RegistryNamedId> worldEquip();

private:
    std::unordered_set<std::string> bodyPrefixes_;
    std::unordered_set<std::string> modelKeys_;
};

} // namespace cube
