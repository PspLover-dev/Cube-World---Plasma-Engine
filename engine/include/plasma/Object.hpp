#pragma once

#include <atomic>
#include <string>

namespace plasma {

class Object {
public:
    Object();
    virtual ~Object();

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;

    void addRef();
    void release();

    int refCount() const { return refCount_.load(); }

protected:
    std::atomic<int> refCount_{1};
};

} // namespace plasma
