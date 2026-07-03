#pragma once

#include "plasma/NamedObject.hpp"
#include "plasma/Math.hpp"

#include <vector>

namespace plasma {

class Engine;
class Display;
class Shape;

class Transformation : public NamedObject {
public:
    Transformation();
    ~Transformation() override;

    Mat4& localMatrix() { return local_; }
    const Mat4& localMatrix() const { return local_; }
    Mat4 worldMatrix() const;

    Transformation* parent() const { return parent_; }
    void setParent(Transformation* parent) { parent_ = parent; }

    Vec3& position() { return position_; }
    Vec3& rotation() { return rotation_; }
    Vec3& scale() { return scale_; }

    void updateMatrix();

private:
    Transformation* parent_{nullptr};
    Mat4 local_{Mat4::identity()};
    Vec3 position_{};
    Vec3 rotation_{};
    Vec3 scale_{1.f, 1.f, 1.f};
};

class Node : public NamedObject {
public:
    explicit Node(Transformation* transform = nullptr);
    ~Node() override;

    Transformation* transform() const { return transform_; }
    void setTransform(Transformation* t) { transform_ = t; }

    void addChild(Node* child);
    const std::vector<Node*>& children() const { return children_; }

    void draw(Engine& engine);

    virtual Shape* asShape() { return nullptr; }

private:
    Transformation* transform_{nullptr};
    std::vector<Node*> children_;
};

} // namespace plasma
