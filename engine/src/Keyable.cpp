#include "plasma/Keyable.hpp"

namespace plasma {

Keyable::~Keyable() = default;

void Keyable::bindKey(int key, KeyHandler handler) {
    bindings_[key] = std::move(handler);
}

void Keyable::unbindKey(int key) {
    bindings_.erase(key);
}

bool Keyable::onKey(int key) {
    const auto it = bindings_.find(key);
    if (it == bindings_.end()) {
        return false;
    }
    if (it->second) {
        it->second();
    }
    return true;
}

} // namespace plasma
