#pragma once

#include "plasma/Math.hpp"
#include "plasma/NamedObject.hpp"

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace plasma {

class Attribute : public Object {
public:
    ~Attribute() override = default;
    virtual const char* typeName() const = 0;
    virtual std::unique_ptr<Attribute> clone() const = 0;
};

template <typename T>
class ContinuousAttribute : public Attribute {
public:
    explicit ContinuousAttribute(T value = T{}) : value_(value) {}
    const char* typeName() const override { return "ContinuousAttribute"; }
    T& value() { return value_; }
    const T& value() const { return value_; }

    std::unique_ptr<Attribute> clone() const override {
        return std::make_unique<ContinuousAttribute<T>>(value_);
    }

    T interpolate(const ContinuousAttribute<T>& other, float t) const {
        if constexpr (std::is_floating_point_v<T>) {
            return value_ + (other.value_ - value_) * t;
        } else {
            return t < 0.5f ? value_ : other.value_;
        }
    }

private:
    T value_;
};

template <typename T>
class ContinuousArrayAttribute : public Attribute {
public:
    std::vector<T>& values() { return values_; }
    const std::vector<T>& values() const { return values_; }
    const char* typeName() const override { return "ContinuousArrayAttribute"; }

    std::unique_ptr<Attribute> clone() const override {
        auto copy = std::make_unique<ContinuousArrayAttribute<T>>();
        copy->values_ = values_;
        return copy;
    }

private:
    std::vector<T> values_;
};

template <typename T>
class DiscreteAttribute : public Attribute {
public:
    explicit DiscreteAttribute(T value = T{}) : value_(value) {}
    const char* typeName() const override { return "DiscreteAttribute"; }
    T& value() { return value_; }
    const T& value() const { return value_; }

    std::unique_ptr<Attribute> clone() const override {
        return std::make_unique<DiscreteAttribute<T>>(value_);
    }

private:
    T value_;
};

using FloatAttribute = ContinuousAttribute<float>;
using IntAttribute = DiscreteAttribute<int>;
using WStringAttribute = DiscreteAttribute<std::wstring>;
using StringAttribute = DiscreteAttribute<std::string>;
using Vec2Attribute = ContinuousAttribute<Vec2>;
using Vec3Attribute = ContinuousAttribute<Vec3>;
using Vec4Attribute = ContinuousAttribute<Vec4>;
using Mat4Attribute = ContinuousAttribute<Mat4>;

using FloatArrayAttribute = ContinuousArrayAttribute<float>;
using Vec2ArrayAttribute = ContinuousArrayAttribute<Vec2>;
using Vec4ArrayAttribute = ContinuousArrayAttribute<Vec4>;

} // namespace plasma
