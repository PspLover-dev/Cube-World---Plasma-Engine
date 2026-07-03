#include "plasma/Node.hpp"
#include "plasma/Shape.hpp"
#include "plasma/Engine.hpp"
#include "plasma/Math.hpp"

namespace plasma {

Transformation::Transformation() {
    local_ = Mat4::identity();
}

Transformation::~Transformation() = default;

Mat4 Transformation::worldMatrix() const {
    Transformation* self = const_cast<Transformation*>(this);
    self->updateMatrix();
    if (parent_) {
        return multiply(parent_->worldMatrix(), local_);
    }
    return local_;
}

void Transformation::updateMatrix() {
    local_ = Mat4::identity();
    local_.m[0] = scale_[0];
    local_.m[5] = scale_[1];
    local_.m[10] = scale_[2];
    local_.m[12] = position_[0];
    local_.m[13] = position_[1];
    local_.m[14] = position_[2];
}

Node::Node(Transformation* transform) : transform_(transform) {}
Node::~Node() = default;

void Node::addChild(Node* child) {
    if (child) {
        children_.push_back(child);
    }
}

void Node::draw(Engine& engine) {
    if (Shape* shape = asShape()) {
        shape->drawShape(engine);
    }
    for (Node* c : children_) {
        if (c) {
            c->draw(engine);
        }
    }
}

} // namespace plasma
