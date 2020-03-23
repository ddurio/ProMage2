#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/Vec2.hpp"


struct AABB2;
struct Plane2;
struct Transform2D;


struct OBB2 {
    public:
    Vec2 center = Vec2::ZERO;
    Vec2 halfExtents = Vec2( 0.5f, 0.5f );
    Vec2 right = Vec2( 1.f, 0.f );
    Vec2 up = Vec2( 0.f, 1.f );


    OBB2() {};
    explicit OBB2( const Vec2& centerIn, const Vec2& halfExtentsIn, const Vec2& rightIn );
    explicit OBB2( const AABB2& aabb2 );
    explicit OBB2( const std::string& textIn );

    void SetFromText( const std::string& textIn );

    bool IsPointInside( const Vec2& point, float radius = 0.f ) const;
    bool IsNearOBB( const OBB2& boxB, float radiusA = 0.f, float radiusB = 0.f ) const;

    Vec2 GetLocalPoint( const Vec2& worldPoint ) const;
    Vec2 GetWorldPoint( const Vec2& localPoint ) const;

    Vec2 GetLocalDirection( const Vec2& worldDirection ) const;
    Vec2 GetWorldDirection( const Vec2& localDirection ) const;

    AABB2 GetLocalBounds() const;
    AABB2 GetBoundingAABB( float paddingRadius = 0.f ) const;

    void GetCorners( Vec2& positionTL, Vec2& positionTR, Vec2& positionBL, Vec2& positionBR ) const;
    void GetCorners( Vec2* positions ) const;
    void GetFaces( Plane2& planeLeft, Plane2& planeRight, Plane2& planeTop, Plane2& planeBot ) const;
    void GetSides( Vec2* sideStarts, Vec2* sideEnds ) const;
    std::string GetAsString() const;

    OBB2 GetTransformed( const Transform2D& transform ) const;
    OBB2 GetTranslated( const Vec2& translation ) const;
    Vec2 GetClosestPoint( const Vec2& point ) const;

    void RotateDegrees( float degrees );
};
