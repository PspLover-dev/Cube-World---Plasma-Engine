#include "plasma/Object.hpp"

namespace plasma {

Object::Object() = default;

Object::~Object() = default;

void Object::addRef() {
    refCount_.fetch_add(1, std::memory_order_relaxed);
}

void Object::release() {
    if (refCount_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        delete this;
    }
}

} // namespace plasma
