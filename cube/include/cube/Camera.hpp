#pragma once

#include "cube/Constants.hpp"
#include "cube/Creature.hpp"
#include "cube/Types.hpp"
#include "plasma/Math.hpp"

namespace cube {

void buildCameraMatrices(const Creature& player, const CameraState& camera, const AnimPose& pose,
                         int fbW, int fbH, plasma::Mat4& outProj, plasma::Mat4& outView);

} // namespace cube
