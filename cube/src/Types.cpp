#include "cube/Types.hpp"

namespace cube {

void EntityState::savePrevious() {
    prevX = posX;
    prevY = posY;
    prevZ = posZ;
}

void EntityState::resetVelocity() {
    velZ = 0.f;
}

plasma::Vec3 partOffset(PartKind kind, const AnimPose& pose) {
    switch (kind) {
    case PartKind::Foot:
        return {pose.body[0] + pose.feet[0], pose.body[1] + pose.feet[1],
                pose.body[2] + pose.feet[2]};
    case PartKind::Head:
        return {pose.body[0] + pose.torso[0], pose.body[1] + pose.torso[1],
                pose.body[2] + pose.torso[2] + 4.f};
    case PartKind::Hand:
        return {pose.body[0], pose.body[1], pose.body[2] + 1.f};
    case PartKind::Body:
    default:
        return pose.body;
    }
}

} // namespace cube
