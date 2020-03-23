#include "Transform3.hpp"

Transform3::Transform3( Vec3 pos /*= Vec3::ZERO*/, float rot /*= 0.f */ ) {
    position = pos;
    rotationDegrees = rot;
}


void Transform3::operator=( const Transform3& copyFrom ) {
    position = copyFrom.position;
    rotationDegrees = copyFrom.rotationDegrees;
}
