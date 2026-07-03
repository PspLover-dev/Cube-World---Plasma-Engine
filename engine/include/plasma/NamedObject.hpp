#pragma once

#include "plasma/Object.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace plasma {

class Widget;
class Texture;
class Font;
class Keyable;
class Shape;

class NamedObject : public Object {
public:
    NamedObject();
    ~NamedObject() override;

    const std::string& name() const { return name_; }
    void setName(std::string name) { name_ = std::move(name); }

    static Widget* createWidget(const std::string& typeName, const std::string& instanceName);
    static Texture* createTexture(const std::string& name);
    static Font* createFont(const std::string& name);
    static Keyable* createKeyable(const std::string& name);
    static Shape* createShape(const std::string& typeName, const std::string& instanceName);

private:
    std::string name_;
};

class ObjectManager {
public:
    void add(NamedObject* obj);
    void remove(NamedObject* obj);
    NamedObject* find(const std::string& name) const;

    template <typename T>
    T* findAs(const std::string& name) const {
        return dynamic_cast<T*>(find(name));
    }

private:
    std::vector<NamedObject*> objects_;
};

} // namespace plasma
