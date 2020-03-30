#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"

struct OBB2;


struct Manifold2 {
    public:
    Vec2 normal = Vec2::ZERO;
    Vec2 contactPos = Vec2::ZERO;
    float penetration = 0.f;

    Manifold2( const Vec2& normalIn = Vec2::ZERO, float penetrationIn = 0.f );
    Manifold2( const std::string& manifoldStr );

    std::string GetAsString() const;
    void SetFromText( const std::string& manifoldStr );

    static bool GetManifold( const OBB2& boxA, const OBB2& boxB, Manifold2& result );
    static bool GetManifold( const OBB2& boxA, float radiusA, const OBB2& boxB, float radiusB, Manifold2& result );
};
