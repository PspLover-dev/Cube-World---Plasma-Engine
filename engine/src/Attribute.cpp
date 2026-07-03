#include "plasma/Attribute.hpp"

// Explicit template instantiations for plasma::Attribute specializations used by the PLX loader.

namespace plasma {

template class ContinuousAttribute<float>;
template class ContinuousAttribute<Vec2>;
template class ContinuousAttribute<Vec3>;
template class ContinuousAttribute<Vec4>;
template class ContinuousAttribute<Mat4>;

template class ContinuousArrayAttribute<float>;
template class ContinuousArrayAttribute<Vec2>;
template class ContinuousArrayAttribute<Vec4>;

template class DiscreteAttribute<int>;
template class DiscreteAttribute<std::string>;
template class DiscreteAttribute<std::wstring>;

} // namespace plasma
