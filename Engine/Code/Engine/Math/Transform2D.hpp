#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"

struct Transform2D {
    public:
    Vec2 position = Vec2::ZERO;
    float rotationDegrees = 0.f;
    //Vec2 scale = Vec2::ONE;

    Transform2D( Vec2 pos = Vec2::ZERO, float rot = 0.f );

    void operator=( const Transform2D& copyFrom );	// Transform2D =  Transform2D
};