#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/Vec3.hpp"


struct Transform3 {
    public:
    Vec3 position = Vec3::ZERO;
    float rotationDegrees = 0.f;
    //Vec2 scale = Vec2::ONE;

    Transform3( Vec3 pos = Vec3::ZERO, float rot = 0.f );

    void operator=( const Transform3& copyFrom );	// Transform2D =  Transform2D
};
