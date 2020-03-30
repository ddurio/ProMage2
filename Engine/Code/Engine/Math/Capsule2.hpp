#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/Vec2.hpp"


struct AABB2;


struct Capsule2 {
    public:
    Vec2 start = Vec2::ZERO;
    Vec2 end = Vec2::ZERO;
    float radius = 0.f;

    Capsule2() {};
    explicit Capsule2( const Vec2& startIn, const Vec2& endIn, float radiusIn );
    explicit Capsule2( const std::string& textIn );

    void operator=( const Capsule2& copyFrom );
    bool operator==( const Capsule2& compareTo ) const;
    bool operator!=( const Capsule2& compareTo ) const;

    void SetFromText( const std::string& textIn );

    std::string GetAsString() const;
    Vec2 GetCenter() const;
    Vec2 GetClosestPoint( const Vec2& point );
    Capsule2 GetTranslated( const Vec2& translation );

    AABB2 GetBoundingAABB() const;

    bool IsPointInside( const Vec2& point ) const;

    void Translate( const Vec2& translation );
};
