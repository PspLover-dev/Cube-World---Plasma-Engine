#include "cube/Camera.hpp"

#include <cmath>

namespace cube {

void buildCameraMatrices(const Creature& player, const CameraState& camera, const AnimPose& pose,
                         int fbW, int fbH, plasma::Mat4& outProj, plasma::Mat4& outView) {
    plasma::setPerspective(outProj, 0.9f, fbW > 0 ? static_cast<float>(fbW) / fbH : 1.f, 0.1f,
                           500.f);

    const EntityState& p = player.state();
    const float targetX = p.posX + p.lookOffsetX + pose.camera[0] * 0.05f;
    const float targetY = p.posY + p.lookOffsetY + pose.camera[1] * 0.05f;
    const float targetZ = p.posZ + kLookHeight + pose.camera[2] * 0.05f;

    const float dist = camera.distance;
    const float cosP = std::cos(camera.pitch);
    const float sinP = std::sin(camera.pitch);
    const float sinY = std::sin(camera.orbitYaw);
    const float cosY = std::cos(camera.orbitYaw);

    const float eyeX = targetX - sinY * cosP * dist;
    const float eyeY = targetY - cosY * cosP * dist;
    const float eyeZ = targetZ + sinP * dist;

    plasma::Vec3 eye{eyeX, eyeY, eyeZ};
    plasma::Vec3 center{targetX, targetY, targetZ};
    plasma::Vec3 up{0.f, 0.f, 1.f};
    plasma::setLookAt(outView, eye, center, up);
}

} // namespace cube
