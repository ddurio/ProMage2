#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/Plane2.hpp"


struct ConvexPoly2;


struct ConvexHull2 {
    std::vector< Plane2 > planes;


    ConvexHull2() {};
    ConvexHull2( const ConvexPoly2& polyToCopy );

    bool operator==( const ConvexHull2& hullB ) const;
    bool operator!=( const ConvexHull2& hullB ) const;

    bool IsPointInside( const Vec2& point ) const;
    bool IsPointInsideIgnorePlane( const Vec2& point, const Plane2& planeToIgnore ) const;
    bool IsPointInsideIgnorePlanes( const Vec2& point, const Plane2& planeToIgnoreA, const Plane2& planeToIgnoreB ) const;
};
