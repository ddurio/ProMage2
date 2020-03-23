#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/Vec3.hpp"


struct AABB2;


struct Capsule3 {
    public:
    Vec3 start = Vec3::ZERO;
    Vec3 end = Vec3::ZERO;
    float radius = 0.f;

    Capsule3() {};
    explicit Capsule3( const Vec3& startIn, const Vec3& endIn, float radiusIn );
    explicit Capsule3( const std::string& textIn );

    void operator=( const Capsule3& copyFrom );
    bool operator==( const Capsule3& compareTo ) const;
    bool operator!=( const Capsule3& compareTo ) const;

    void SetFromText( const std::string& textIn );

    std::string GetAsString() const;
    Vec3 GetCenter() const;
    Vec3 GetClosestPoint( const Vec3& point );
    Capsule3 GetTranslated( const Vec3& translation );

    //AABB3 GetBoundingAABB() const;

    bool IsPointInside( const Vec3& point ) const;

    void Translate( const Vec3& translation );
};
