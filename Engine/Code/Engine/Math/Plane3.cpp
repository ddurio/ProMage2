#include "Engine/Math/Plane3.hpp"

#include "Engine/Math/MathUtils.hpp"


Plane3::Plane3( const Vec3& normalIn, float distanceIn ) :
    normal( normalIn ),
    distance( distanceIn ) {
}


Plane3::Plane3( const Vec3& pointA, const Vec3& pointB, const Vec3& pointC ) {
    Vec3 aToB = pointB - pointA;
    Vec3 aToC = pointC - pointA;

    normal = CrossProduct( aToB, aToC );
    distance = DotProduct( normal, pointA );
}


float Plane3::GetDistanceFromPlane( const Vec3& point ) const {
    float projection = DotProduct( normal, point );
    return projection - distance;
}
