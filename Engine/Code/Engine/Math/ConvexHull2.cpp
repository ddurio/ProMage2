#include "Engine/Math/ConvexHull2.hpp"

#include "Engine/Math/ConvexPoly2.hpp"
#include "Engine/Math/MathUtils.hpp"


ConvexHull2::ConvexHull2( const ConvexPoly2& polyToCopy ) {
    int numVerts = (int)polyToCopy.positions.size();
    GUARANTEE_OR_DIE( numVerts >= 3, "(ConvexShape) ERROR -- Not enough verts exist in poly" );

    for( int vertIndex = 0; vertIndex < numVerts - 1; vertIndex++ ) {
        Plane2 shapeSide = Plane2( polyToCopy.positions[vertIndex], polyToCopy.positions[vertIndex + 1] );
        planes.push_back( shapeSide );
    }

    Plane2 finalSide = Plane2( polyToCopy.positions[numVerts - 1], polyToCopy.positions[0] );
    planes.push_back( finalSide );
}


bool ConvexHull2::operator==( const ConvexHull2& hullB ) const {
    return (planes == hullB.planes);
}


bool ConvexHull2::operator!=( const ConvexHull2& hullB ) const {
    return (planes != hullB.planes);
}


bool ConvexHull2::IsPointInside( const Vec2& point ) const {
    int numPlanes = (int)planes.size();

    for( int planeIndex = 0; planeIndex < numPlanes; planeIndex++ ) {
        const Plane2& plane = planes[planeIndex];
        float distFromPlane = plane.GetDistanceFromPlane( point );

        if( distFromPlane > 0.f ) {
            return false;
        }
    }

    return true;
}


bool ConvexHull2::IsPointInsideIgnorePlane( const Vec2& point, const Plane2& planeToIgnore ) const {
    int numPlanes = (int)planes.size();

    for( int planeIndex = 0; planeIndex < numPlanes; planeIndex++ ) {
        const Plane2& plane = planes[planeIndex];

        if( &plane == &planeToIgnore ) { // Ignore if plane pointers match
            continue;
        }

        float distFromPlane = plane.GetDistanceFromPlane( point );

        if( distFromPlane > 0.f ) {
            return false;
        }
    }

    return true;
}


bool ConvexHull2::IsPointInsideIgnorePlanes( const Vec2& point, const Plane2& planeToIgnoreA, const Plane2& planeToIgnoreB ) const {
    int numPlanes = (int)planes.size();

    for( int planeIndex = 0; planeIndex < numPlanes; planeIndex++ ) {
        const Plane2& plane = planes[planeIndex];

        if( &plane == &planeToIgnoreA || &plane == &planeToIgnoreB ) { // Ignore if plane pointers match
            continue;
        }

        float distFromPlane = plane.GetDistanceFromPlane( point );

        if( distFromPlane > 0.f ) {
            return false;
        }
    }

    return true;
}
