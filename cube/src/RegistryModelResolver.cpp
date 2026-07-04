#include "cube/RegistryModelResolver.hpp"

#include "cube/Assets.hpp"

#include <algorithm>
#include <cctype>
#include <unordered_map>

#include "RegistryModelOverrides.inc"

namespace cube {

namespace {

std::string pascalToKebab(const std::string& name) {
    std::string out;
    for (size_t i = 0; i < name.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(name[i]);
        if (std::isupper(c) != 0 && i > 0) {
            const unsigned char prev = static_cast<unsigned char>(name[i - 1]);
            const unsigned char next =
                i + 1 < name.size() ? static_cast<unsigned char>(name[i + 1]) : 0;
            if (std::islower(prev) != 0 || (std::isupper(prev) != 0 && std::islower(next) != 0)) {
                out.push_back('-');
            }
        }
        out.push_back(static_cast<char>(std::tolower(c)));
    }
    return out;
}

std::string stripHyphens(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c != '-') {
            out.push_back(c);
        }
    }
    return out;
}

} // namespace

std::vector<RegistryNamedId> RegistryModelResolver::worldRaces() {
    std::vector<RegistryNamedId> races;
#define REGISTRY_RACE(id, name) \
    races.push_back(RegistryNamedId{static_cast<uint32_t>(id), 0u, name})
#include "WorldRaceRegistry.inc"
#undef REGISTRY_RACE
    return races;
}

std::vector<RegistryNamedId> RegistryModelResolver::worldItems() {
    std::vector<RegistryNamedId> items;
#define registerItem(id, name) \
    items.push_back(RegistryNamedId{static_cast<uint32_t>(id), 0u, name})
#include "WorldItems.inc"
#undef registerItem
    return items;
}

std::vector<RegistryNamedId> RegistryModelResolver::worldEquip() {
    std::vector<RegistryNamedId> equip;
#define REGISTRY_EQUIP(slot, sub, name) \
    equip.push_back(RegistryNamedId{static_cast<uint32_t>(slot), static_cast<uint32_t>(sub), name})
#include "WorldEquipRegistry.inc"
#undef REGISTRY_EQUIP
    return equip;
}

void RegistryModelResolver::rebuild(const ModelCatalog& catalog) {
    bodyPrefixes_.clear();
    modelKeys_.clear();

    for (const std::string& key : catalog.modelNames()) {
        modelKeys_.insert(key);
        const std::string bodySuffix = "-body.cub";
        if (key.size() > bodySuffix.size() &&
            key.compare(key.size() - bodySuffix.size(), bodySuffix.size(), bodySuffix) == 0) {
            bodyPrefixes_.insert(key.substr(0, key.size() - bodySuffix.size()));
        }
    }
}

std::optional<RegistryCreatureModel> RegistryModelResolver::resolveCreature(
    const std::string& registryName) const {
    std::vector<std::string> candidates;
    if (auto it = registry_model_detail::kCreatureOverrides.find(registryName);
        it != registry_model_detail::kCreatureOverrides.end()) {
        candidates.push_back(it->second);
    }

    const std::string kebab = pascalToKebab(registryName);
    candidates.push_back(kebab);
    candidates.push_back(stripHyphens(kebab));

    const std::string norm = stripHyphens(kebab);
    for (const std::string& prefix : bodyPrefixes_) {
        if (stripHyphens(prefix) == norm) {
            candidates.push_back(prefix);
        }
    }

    for (const std::string& prefix : candidates) {
        if (bodyPrefixes_.count(prefix) != 0) {
            return RegistryCreatureModel{prefix, true};
        }
    }
    for (const std::string& prefix : candidates) {
        const std::string key = prefix + ".cub";
        if (modelKeys_.count(key) != 0) {
            return RegistryCreatureModel{prefix, false};
        }
    }
    return std::nullopt;
}

std::optional<RegistryEquipModel> RegistryModelResolver::resolveEquip(
    const std::string& registryName) const {
    std::vector<std::string> candidates;
    if (auto it = registry_model_detail::kEquipOverrides.find(registryName);
        it != registry_model_detail::kEquipOverrides.end()) {
        candidates.push_back(it->second);
    }

    const std::string kebab = pascalToKebab(registryName);
    candidates.push_back(kebab);
    candidates.push_back(kebab + "1");
    candidates.push_back("iron-" + kebab);

    for (const std::string& stem : candidates) {
        const std::string key = stem + ".cub";
        if (modelKeys_.count(key) != 0) {
            return RegistryEquipModel{key};
        }
    }
    return std::nullopt;
}

} // namespace cube
