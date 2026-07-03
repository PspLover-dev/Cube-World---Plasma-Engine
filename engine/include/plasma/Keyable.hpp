#pragma once

#include "plasma/NamedObject.hpp"

#include <functional>
#include <unordered_map>

namespace plasma {

/// Named keyboard shortcut target (plasma::Keyable).
class Keyable : public NamedObject {
public:
    using NamedObject::NamedObject;
    ~Keyable() override;

    using KeyHandler = std::function<void()>;

    void bindKey(int key, KeyHandler handler);
    void unbindKey(int key);
    bool onKey(int key);

    const std::unordered_map<int, KeyHandler>& bindings() const { return bindings_; }

private:
    std::unordered_map<int, KeyHandler> bindings_;
};

} // namespace plasma
