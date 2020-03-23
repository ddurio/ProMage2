#include "Transform2D.hpp"

Transform2D::Transform2D( Vec2 pos /*= Vec2::ZERO*/, float rot /*= 0.f */ ) {
    position = pos;
    rotationDegrees = rot;
}


void Transform2D::operator=( const Transform2D& copyFrom ) {
    position = copyFrom.position;
    rotationDegrees = copyFrom.rotationDegrees;
}