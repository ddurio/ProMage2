#include "Engine/Math/Plane2.hpp"

#include "Engine/Math/MathUtils.hpp"


Plane2::Plane2( const Vec2& normalIn, float distanceIn ) :
    normal( normalIn ),
    distance( distanceIn ) {
}


// Will always make normal to the right of line from A to B
Plane2::Plane2( const Vec2& startPoint, const Vec2& endPoint ) {
    Vec2 tangent = endPoint - startPoint;
    normal = tangent.GetNormalized().GetRotatedMinus90Degrees();
    distance = DotProduct( normal, startPoint );
}


float Plane2::GetDistanceFromPlane( const Vec2& point ) const {
    float projection = DotProduct( normal, point );
    return projection - distance;
}
