#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Vertex/VertexUtils.hpp"


struct ConvexHull2;


struct ConvexPoly2 {
    std::vector< Vec2 > positions; // Should be strictly positive theta ordered (CCW normally)


    ConvexPoly2() {};
    ConvexPoly2( const ConvexHull2& hullToCopy );

    bool operator==( const ConvexPoly2& polyB ) const;
    bool operator!=( const ConvexPoly2& polyB ) const;

    VertexList GetVertexList( const Rgba& tint = Rgba::WHITE ) const;
};