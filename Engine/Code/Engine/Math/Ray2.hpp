#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/Vec2.hpp"


struct AABB2;
//struct Capsule3;
//struct OBB3;
struct Plane2;


struct Ray2 {
    public:
    Vec2 start = Vec2::ZERO;
    Vec2 direction = Vec2::ZERO;

    explicit Ray2( const Vec2& startIn, const Vec2& directionIn );
    explicit Ray2( const std::string& valuesAsText );
    static Ray2 MakeFromPoints( const Vec2& start, const Vec2& end );

    bool operator==( const Ray2& otherRay ) const;

    void SetFromText( const std::string& valuesAsText );
    std::string GetAsString() const;

    Vec2 GetPoint( float distanceAlong ) const;

    //int Raycast( const Vec2& discCenter, float discRadius, float* outIntersections ) const;
    //int Raycast( const Capsule3& capsule, float* outIntersections ) const;
    int Raycast( const Plane2& plane, float* outIntersections ) const;
    int Raycast( const AABB2& box, float* outIntersections ) const;
    //int Raycast( const OBB3& box, float* outIntersections ) const;
    //int Raycast( const Vec2& cylinderStart, const Vec2& cylinderEnd, float radius, float* outIntersections ) const;

    float GetDistanceAlongAtX( float xValue ) const;
    float GetDistanceAlongAtY( float yValue ) const;
};
