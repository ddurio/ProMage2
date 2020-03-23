#include "Engine/Math/OBB3.hpp"


/*
void OBB3::RotateByDegrees( float degrees ) {

}


bool OBB3::IsPointInside( Vec3 worldPoint ) {

}


Vec3 OBB3::WorldToLocal( Vec3 worldPoint ) {

}
*/


OBB3::OBB3( const Vec3& centerIn, const Vec3& halfExtentsIn, const Vec3& rightIn, const Vec3& upIn ) :
    center(centerIn),
    halfExtents(halfExtentsIn),
    right(rightIn),
    up(upIn) {
}
