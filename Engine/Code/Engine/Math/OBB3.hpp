#pragma once

#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/Vec3.hpp"


struct OBB3 {
    public:
    // Defaults to 1x1x1 cube at origin
    Vec3 center = Vec3::ZERO;
    Vec3 right = Vec3( 1.f, 0.f, 0.f );
    Vec3 up = Vec3( 0.f, 1.f, 0.f );
    Vec3 halfExtents = Vec3( 0.5f, 0.5f, 0.5f );

    explicit OBB3( const Vec3& centerIn, const Vec3& halfExtentsIn, const Vec3& rightIn, const Vec3& upIn );

    void RotateByDegrees( float degrees );
    bool IsPointInside( Vec3 worldPoint );
    Vec3 WorldToLocal( Vec3 worldPoint );
};
