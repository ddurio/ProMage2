#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/Vec2.hpp"


struct Plane2 {
    public:
    Vec2 normal = Vec2::UP;
    float distance = 0.f;

    Plane2() {};
    Plane2( const Vec2& normalIn, float distanceIn );
    Plane2( const Vec2& pointA, const Vec2& pointB );

    float GetDistanceFromPlane( const Vec2& point ) const;
};
