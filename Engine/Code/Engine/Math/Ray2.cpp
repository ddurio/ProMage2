#include "Engine/Math/Ray2.hpp"

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Plane2.hpp"


Ray2::Ray2( const Vec2& startIn, const Vec2& directionIn ) :
    start( startIn ),
    direction( directionIn.GetNormalized() ) {
}


Ray2::Ray2( const std::string& valuesAsText ) {
    SetFromText( valuesAsText );
}


Ray2 Ray2::MakeFromPoints( const Vec2& start, const Vec2& end ) {
    Vec2 dir = (end - start).GetNormalized();
    Ray2 ray = Ray2( start, dir );

    return ray;
}


bool Ray2::operator==( const Ray2& otherRay ) const {
    if( start       == otherRay.start &&
        direction   == otherRay.direction ) {
        return true;
    }

    return false;
}


void Ray2::SetFromText( const std::string& valuesAsText ) {
    // Expects "startX,startY,startZ dirX,dirY,dirZ"
    std::vector<std::string> splitString = SplitStringOnDelimeter( valuesAsText, ' ' );

    int numValues = (int)splitString.size();
    GUARANTEE_OR_DIE( numValues == 2, Stringf( "(Ray2) Invalid initialization string (%s)", valuesAsText.c_str() ) );

    start = Vec2( splitString[0] );
    direction = Vec2( splitString[1] ).GetNormalized();
}


std::string Ray2::GetAsString() const {
    std::string startStr = start.GetAsString();
    std::string dirStr = direction.GetAsString();
    std::string rayStr = Stringf( "%s %s", startStr.c_str(), dirStr.c_str() );

    return rayStr;
}


Vec2 Ray2::GetPoint( float distanceAlong ) const {
    return start + (direction * distanceAlong);
}


int Ray2::Raycast( const Plane2& plane, float* outIntersections ) const {
    float distFromPlane = plane.GetDistanceFromPlane( start );
    float alignmentWithPlane = DotProduct( direction, plane.normal );

    if( alignmentWithPlane == 0.f ) { // Parallel to plane
        return 0;
    }

    float impactTime = distFromPlane / (-alignmentWithPlane);

    if( impactTime < 0.f ) { // Impacts behind ray
        return 0;
    }

    *outIntersections = impactTime;
    return 1;
}


int Ray2::Raycast( const AABB2& box, float* outIntersections ) const {
    // Matches min/max of AABB2
    float timeAtMinX = GetDistanceAlongAtX( box.mins.x );
    float timeAtMaxX = GetDistanceAlongAtX( box.maxs.x );

    float timeAtMinY = GetDistanceAlongAtY( box.mins.y );
    float timeAtMaxY = GetDistanceAlongAtY( box.maxs.y );

    // Matches min/max along Ray
    float minTimeX = Min( timeAtMinX, timeAtMaxX );
    float maxTimeX = Max( timeAtMinX, timeAtMaxX );

    float minTimeY = Min( timeAtMinY, timeAtMaxY );
    float maxTimeY = Max( timeAtMinY, timeAtMaxY );

    // Check for overlap
    float minTime = Max( minTimeX, minTimeY );
    minTime       = Max( minTime, 0.f ); // don't hit things behind the ray
    float maxTime = Min( maxTimeX, maxTimeY );

    if( minTime < maxTime ) {
        outIntersections[0] = minTime;
        outIntersections[1] = maxTime;
        return 2;
    } else if( minTime == maxTime ) {
        outIntersections[0] = minTime;
        return 1;
    }

    return 0;
}


float Ray2::GetDistanceAlongAtX( float xValue ) const {
    if( direction.x == 0.f ) {
        return std::numeric_limits<float>::infinity();
    }

    float distAlong = (xValue - start.x) / direction.x;
    return distAlong;
}


float Ray2::GetDistanceAlongAtY( float yValue ) const {
    if( direction.y == 0.f ) {
        return std::numeric_limits<float>::infinity();
    }

    float distAlong = (yValue - start.y) / direction.y;
    return distAlong;
}
