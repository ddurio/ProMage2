#include "Engine/Math/ConvexPoly2.hpp"

#include "Engine/Math/ConvexHull2.hpp"
#include "Engine/Math/MathUtils.hpp"


ConvexPoly2::ConvexPoly2( const ConvexHull2& hullToCopy ) {
    std::vector< Vec2 > unorderedVerts;
    int numPlanes = (int)hullToCopy.planes.size();

    // Plane 0 to second to last (looking at plane pairs)
    for( int planeIndexA = 0; planeIndexA < numPlanes - 1; planeIndexA++ ) {
        const Plane2& planeA = hullToCopy.planes[planeIndexA];
        Vec2 pointOnA = planeA.normal * planeA.distance;
        Vec2 tangentOfA = planeA.normal.GetRotated90Degrees();

        // Pseudo-Bubble sort style (plane one after first loop to end)
        for( int planeIndexB = planeIndexA + 1; planeIndexB < numPlanes; planeIndexB++ ) {
            const Plane2& planeB = hullToCopy.planes[planeIndexB];

            // Use planeA like a Ray (adapted from Ray2 vs Plane2 raycast)
            float distPointAToPlaneB = planeB.GetDistanceFromPlane( pointOnA );
            float alignmentWithPlaneB = DotProduct( tangentOfA, planeB.normal );

            if( alignmentWithPlaneB == 0.f ) { // Parallel planes
                continue;
            }

            float intersectionTime = distPointAToPlaneB / (-alignmentWithPlaneB);
            Vec2 intersectionPoint = pointOnA + (intersectionTime * tangentOfA);

            // Validate intersection point
            if( hullToCopy.IsPointInsideIgnorePlanes( intersectionPoint, planeA, planeB ) ) {
                unorderedVerts.push_back( intersectionPoint );
            }
        }
    }

    int numVerts = (int)unorderedVerts.size();
    GUARANTEE_RECOVERABLE( numVerts == numPlanes, "(ConvexPoly2) WARNING -- Shouldn't these match sizes?" );

    if( numVerts <= 2 ) { // Should always be at least three...
        positions = unorderedVerts;
        return;
    }

    // Sort the verts
    positions.reserve( numVerts );
    positions.push_back( unorderedVerts[0] );
    positions.push_back( unorderedVerts[1] );

    for( int vertIndex = 2; vertIndex < numVerts; vertIndex++ ) {
        Vec2 newPosition = unorderedVerts[vertIndex];
        int numPositions = (int)positions.size();
        bool addedNewPos = false;

        for( int posIndex = 0; posIndex < numPositions - 1; posIndex++ ) {
            Vec2 posA = positions[posIndex];
            Vec2 posB = positions[posIndex + 1];
            Plane2 polyEdge = Plane2( posA, posB );

            if( polyEdge.IsPointInFront( newPosition ) ) {
                std::vector< Vec2 >::iterator insertIter = positions.begin() + posIndex + 1;
                positions.insert( insertIter, newPosition );
                addedNewPos = true;
                break;
            }
        }

        if( !addedNewPos ) {
            positions.push_back( newPosition );
        }
    }
}


bool ConvexPoly2::operator==( const ConvexPoly2& polyB ) const {
    return (positions == polyB.positions);
}


bool ConvexPoly2::operator!=( const ConvexPoly2& polyB ) const {
    return (positions != polyB.positions);
}


VertexList ConvexPoly2::GetVertexList( const Rgba& tint /*= Rgba::WHITE */ ) const {
    VertexList vertList;
    int numPositions = (int)positions.size();
    GUARANTEE_OR_DIE( numPositions >= 3, "(ConvexPoly2) ERROR -- Not enough verts exist to close poly" );

    Vec3 startPos3 = Vec3( positions[0], 0.f );
    VertexPCU startVert = VertexPCU( startPos3, tint, Vec2::ZERO );

    Vec3 secondPos3 = Vec3( positions[1], 0.f );
    VertexPCU prevVert = VertexPCU( secondPos3, tint, Vec2::ZERO );

    for( int posIndex = 2; posIndex < numPositions; posIndex++ ) {
        Vec3 currPos3 = Vec3( positions[posIndex], 0.f );
        VertexPCU currVert = VertexPCU( currPos3, tint, Vec2::ZERO );

        vertList.push_back( startVert );
        vertList.push_back( currVert );
        vertList.push_back( prevVert );

        prevVert = currVert;
    }

    return vertList;
}