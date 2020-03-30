#include "Engine/Math/OBB2.hpp"

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Plane2.hpp"
#include "Engine/Math/Transform2D.hpp"


OBB2::OBB2( const Vec2& centerIn, const Vec2& halfExtentsIn, const Vec2& rightIn ) :
    center(centerIn),
    halfExtents(halfExtentsIn),
    right(rightIn.GetNormalized()) {
    up = right.GetRotated90Degrees();
}


OBB2::OBB2( const AABB2& aabb2 ) {
    center = aabb2.GetCenter();
    halfExtents = aabb2.GetDimensions() * 0.5f;
    right = Vec2::RIGHT;
    up = Vec2::UP;
}


OBB2::OBB2( const std::string& textIn ) {
    SetFromText( textIn );
}


void OBB2::SetFromText( const std::string& textIn ) {
    Strings params = SplitStringOnDelimeter( textIn, ' ' );
    int numParams = (int)params.size();
    GUARANTEE_OR_DIE( numParams == 3, Stringf( "Invalid OBB2 setFromText value (%s)", textIn.c_str() ) );

    center = Vec2( params[0] );
    halfExtents = Vec2( params[1] );
    right = Vec2( params[2] ).GetNormalized();
    up = right.GetRotated90Degrees();
}


bool OBB2::IsPointInside( const Vec2& point, float radius /*= 0.f*/ ) const {
    Vec2 localPoint = GetLocalPoint( point );

    if( (abs( localPoint.x ) < halfExtents.x) && (abs( localPoint.y ) < halfExtents.y) ) {
        return true;
    }

    Vec2 closestPoint;
    closestPoint.x = Clamp( localPoint.x, -halfExtents.x, halfExtents.x );
    closestPoint.y = Clamp( localPoint.y, -halfExtents.y, halfExtents.y );

    Vec2 displacement = closestPoint - localPoint;
    return (displacement.GetLengthSquared() < radius * radius);
}


bool OBB2::IsNearOBB( const OBB2& boxB, float radiusA /*= 0.f*/, float radiusB /*= 0.f */ ) const {
    float discRadiusA = halfExtents.x + halfExtents.y + radiusA;
    float discRadiusB = boxB.halfExtents.x + boxB.halfExtents.y + radiusB;

    return DoDiscsOverlap( center, discRadiusA, boxB.center, discRadiusB );
}


Vec2 OBB2::GetLocalPoint( const Vec2& worldPoint ) const {
    Vec2 displacement = worldPoint - center;
    float localI = DotProduct( displacement, right );
    float localJ = DotProduct( displacement, up );
    return Vec2( localI, localJ );
}


Vec2 OBB2::GetWorldPoint( const Vec2& localPoint ) const {
    return center + (localPoint.x * right) + (localPoint.y * up);
}


void OBB2::GetCorners( Vec2& positionTL, Vec2& positionTR, Vec2& positionBL, Vec2& positionBR ) const {
    positionTL = center + halfExtents.y * up - halfExtents.x * right;
    positionTR = center + halfExtents.y * up + halfExtents.x * right;
    positionBL = center - halfExtents.y * up - halfExtents.x * right;
    positionBR = center - halfExtents.y * up + halfExtents.x * right;
}


void OBB2::GetCorners( Vec2* positions ) const {
    GetCorners( positions[0], positions[1], positions[2], positions[3] );
}


void OBB2::GetFaces( Plane2& planeLeft, Plane2& planeRight, Plane2& planeTop, Plane2& planeBot ) const {
    planeLeft  = Plane2( -right, DotProduct( -right, center - (halfExtents.x * right) ) );
    planeRight = Plane2(  right, DotProduct(  right, center + (halfExtents.x * right) ) );
    planeTop   = Plane2(  up,    DotProduct(  up,    center + (halfExtents.y * up) ) );
    planeBot   = Plane2( -up,    DotProduct( -up,    center - (halfExtents.y * up) ) );
}


void OBB2::GetSides( Vec2* sideStarts, Vec2* sideEnds ) const {
    Vec2 topLeft;
    Vec2 topRight;
    Vec2 botLeft;
    Vec2 botRight;
    GetCorners( topLeft, topRight, botLeft, botRight );

    sideStarts[0] = botLeft;
    sideStarts[1] = topRight;
    sideStarts[2] = topLeft;
    sideStarts[3] = botRight;

    sideEnds[0] = topLeft;
    sideEnds[1] = botRight;
    sideEnds[2] = topRight;
    sideEnds[3] = botLeft;
}


std::string OBB2::GetAsString() const {
    std::string centerStr = center.GetAsString();
    std::string extentsStr = halfExtents.GetAsString();
    std::string rightStr = right.GetAsString();

    std::string boxStr = Stringf( "%s %s %s", centerStr.c_str(), extentsStr.c_str(), rightStr.c_str() );
    return boxStr;
}


OBB2 OBB2::GetTransformed( const Transform2D& transform ) const {
    OBB2 rotated = *this;
    rotated.RotateDegrees( transform.rotationDegrees );

    return rotated.GetTranslated( transform.position );
}


OBB2 OBB2::GetTranslated( const Vec2& translation ) const {
    OBB2 translated = *this;
    translated.center += translation;
    return translated;
}


Vec2 OBB2::GetClosestPoint( const Vec2& point ) const {
    Vec2 localPoint = GetLocalPoint( point );
    localPoint.x = Clamp( localPoint.x, -halfExtents.x, halfExtents.x );
    localPoint.y = Clamp( localPoint.y, -halfExtents.y, halfExtents.y );

    return GetWorldPoint( localPoint );
}


void OBB2::RotateDegrees( float degrees ) {
    right.RotateDegrees( degrees );
    up = right.GetRotated90Degrees();
}
