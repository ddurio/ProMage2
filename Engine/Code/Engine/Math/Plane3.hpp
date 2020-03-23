#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/Vec3.hpp"


struct Plane3 {
    public:
    Vec3 normal = Vec3::UP;
    float distance = 0.f;

    Plane3() {};
    Plane3( const Vec3& normalIn, float distanceIn );
    Plane3( const Vec3& pointA, const Vec3& pointB, const Vec3& pointC ); // Assumes clockwise, left-handed winding order

    float GetDistanceFromPlane( const Vec3& point ) const;
};
