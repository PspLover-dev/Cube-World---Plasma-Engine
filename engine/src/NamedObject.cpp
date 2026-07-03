#include "plasma/NamedObject.hpp"
#include "plasma/Widget.hpp"
#include "plasma/Font.hpp"
#include "plasma/Keyable.hpp"
#include "plasma/Shape.hpp"
#include "plasma/Node.hpp"

#include <algorithm>

namespace plasma {

NamedObject::NamedObject() = default;
NamedObject::~NamedObject() = default;

Widget* NamedObject::createWidget(const std::string& typeName, const std::string& instanceName) {
  Widget* w = nullptr;
  if (typeName == "Button" || typeName == "button") {
    w = new Button();
  } else if (typeName == "PopUpButton") {
    w = new PopUpButton();
  } else if (typeName == "ListWidget" || typeName == "list") {
    w = new ListWidget();
  } else if (typeName == "Edit") {
    w = new Edit();
  } else if (typeName == "ScrollButton") {
    w = new ScrollButton();
  } else if (typeName == "ScrollSlider") {
    w = new ScrollSlider();
  } else if (typeName == "ScrollBar") {
    w = new ScrollBar();
  } else {
    w = new Widget();
  }
  w->setName(instanceName);
  return w;
}

Texture* NamedObject::createTexture(const std::string& name) {
  (void)name;
  return nullptr;
}

Font* NamedObject::createFont(const std::string& name) {
  (void)name;
  return new PlasmaFont();
}

Keyable* NamedObject::createKeyable(const std::string& name) {
  auto* k = new Keyable();
  k->setName(name);
  return k;
}

Shape* NamedObject::createShape(const std::string& typeName, const std::string& instanceName) {
    Shape* s = nullptr;
    if (typeName == "GenericShape") {
        s = new GenericShape();
    } else if (typeName == "MeshShape") {
        s = new MeshShape();
    } else if (typeName == "StaticMeshShape") {
        s = new StaticMeshShape();
    } else if (typeName == "SmoothMeshShape") {
        s = new SmoothMeshShape();
    } else if (typeName == "CurveShape") {
        s = new CurveShape();
    } else if (typeName == "TextShape") {
        s = new TextShape();
    } else {
        s = new Shape();
    }
    s->setName(instanceName);
    return s;
}

void ObjectManager::add(NamedObject* obj) {
  objects_.push_back(obj);
}

void ObjectManager::remove(NamedObject* obj) {
  objects_.erase(std::remove(objects_.begin(), objects_.end(), obj), objects_.end());
}

NamedObject* ObjectManager::find(const std::string& name) const {
  for (NamedObject* o : objects_) {
    if (o && o->name() == name) {
      return o;
    }
  }
  return nullptr;
}

} // namespace plasma
