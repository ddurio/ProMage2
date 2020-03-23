#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/Vec3.hpp"


struct Capsule3;
struct OBB3;
struct Plane3;


struct Ray3 {
    public:
    Vec3 start = Vec3::ZERO;
    Vec3 direction = -Vec3::SCREEN;

    explicit Ray3( const Vec3& startIn = Vec3::ZERO, const Vec3& directionIn = -Vec3::SCREEN );
    explicit Ray3( const std::string& valuesAsText );
    static Ray3 MakeFromPoints( const Vec3& start, const Vec3& end );

    void SetFromText( const std::string& valuesAsText );
    std::string GetAsString() const;

    Vec3 GetPoint( float distanceAlong ) const;

    int Raycast( const Vec3& sphereCenter, float sphereRadius, float* outIntersections ) const;
    int Raycast( const Capsule3& capsule, float* outIntersections ) const;
    int Raycast( const Plane3& plane, float* outIntersections ) const;
    int Raycast( const OBB3& box, float* outIntersections ) const;
    int Raycast( const Vec3& cylinderStart, const Vec3& cylinderEnd, float radius, float* outIntersections ) const;
};
