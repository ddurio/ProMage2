#include "Engine/Physics/Manifold.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Plane2.hpp"

#include "algorithm"


Manifold2::Manifold2( const Vec2& normalIn /*= Vec2::ZERO*/, float penetrationIn /*= 0.f*/) :
    normal(normalIn),
    penetration(penetrationIn) {
}


Manifold2::Manifold2( const std::string& manifoldStr ) {
    SetFromText( manifoldStr );
}


std::string Manifold2::GetAsString() const {
    std::string normalStr = normal.GetAsString();
    std::string manifoldStr = Stringf( "%s %.2f", normalStr.c_str(), penetration );

    return manifoldStr;
}


void Manifold2::SetFromText( const std::string& manifoldStr ) {
    Strings params = SplitStringOnDelimeter( manifoldStr, ' ' );
    int numParams = (int)params.size();

    GUARANTEE_OR_DIE( numParams == 2, Stringf( "(Manifold2) Invalid setFromText input: %s", manifoldStr.c_str() ) );
    
    normal = Vec2( params[0] );
    penetration = (float)atof( params[1].c_str() );
}


bool Manifold2::GetManifold( const OBB2& boxA, const OBB2& boxB, Manifold2& result ) {
    Plane2 faces[8];
    boxA.GetFaces( faces[0], faces[1], faces[2], faces[3] );
    boxB.GetFaces( faces[4], faces[5], faces[6], faces[7] );

    Vec2 corners[8]; // Reversed order (compare BoxA faces with BoxB corners)
    boxB.GetCorners( corners[0], corners[1], corners[2], corners[3] );
    boxA.GetCorners( corners[4], corners[5], corners[6], corners[7] );

    float worstCases[8];
    int worstCaseIndexes[8];

    for( int boxIndex = 0; boxIndex < 2; boxIndex++ ) { // BoxA, BoxB
        for( int boxFaceIndex = 0; boxFaceIndex < 4; boxFaceIndex++ ) { // Left, right, top, bot
            int faceIndex = 4 * boxIndex + boxFaceIndex;
            const Plane2& face = faces[faceIndex];
            float worstCase = 1.f;
            int worstCaseIndex = -1;

            for( int boxCornerIndex = 0; boxCornerIndex < 4; boxCornerIndex++ ) { // TL, TR, BL, BR
                int cornerIndex = 4 * boxIndex + boxCornerIndex;
                const Vec2& corner = corners[cornerIndex];
                float distFromPlane = face.GetDistanceFromPlane( corner );

                if( distFromPlane < worstCase ) {
                    worstCase = distFromPlane;
                    worstCaseIndex = cornerIndex;
                }
            }

            if( worstCase > 0.f ) {
                return false;
            } else {
                worstCases[faceIndex] = worstCase;
                worstCaseIndexes[faceIndex] = worstCaseIndex;
            }
        }
    }

    int bestCaseIndex = 0; // Assume first options is best so far
    int alignedEdgeIndex = -1;
    float bestCase = worstCases[0];
    float epsilon = 0.00001f;

    for( int caseIndex = 1; caseIndex < 8; caseIndex++ ) {
        float distance = worstCases[caseIndex];
        float difference = abs( distance - bestCase );

        if( (distance > bestCase) && (difference > epsilon) ) { // Greater than means less negative in this case
            bestCaseIndex = caseIndex;
            bestCase = distance;
            alignedEdgeIndex = -1;
        } else if( difference < epsilon ) {
            alignedEdgeIndex = caseIndex;
        }
    }

    // Normal
    result.normal = faces[bestCaseIndex].normal;

    if( bestCaseIndex < 4 ) { // Face is on BoxA.. need to invert the normal
        result.normal = -result.normal;
    }

    // Contact Point
    int contactIndex = worstCaseIndexes[bestCaseIndex];

    if( alignedEdgeIndex != -1 ) {
        Vec2 sideStarts[8];
        Vec2 sideEnds[8];
        boxA.GetSides( sideStarts, sideEnds );
        boxB.GetSides( &sideStarts[4], &sideEnds[4] );

        Vec2 lineStart = sideStarts[bestCaseIndex];
        Vec2 lineEnd = sideEnds[bestCaseIndex];

        Vec2 alignedEdgeStart = sideStarts[alignedEdgeIndex];
        Vec2 alignedEdgeEnd = sideEnds[alignedEdgeIndex];

        Vec2 contactStart = GetClosestPointOnLineSegment( alignedEdgeStart, lineStart, lineEnd );
        Vec2 contactEnd = GetClosestPointOnLineSegment( alignedEdgeEnd, lineStart, lineEnd );

        result.contactPos = (contactStart + contactEnd) * 0.5f;
    } else {
        result.contactPos = corners[contactIndex];
    }

    // Penetration
    result.penetration = -bestCase;
    result.penetration += 0.000001f; // Adjust for floating point precision... make for damn sure they're separated

    return true;
}


bool Manifold2::GetManifold( const OBB2& boxA, float radiusA, const OBB2& boxB, float radiusB, Manifold2& result ) {
    // Disc proximity check first
    if( !boxA.IsNearOBB( boxB, radiusA, radiusB ) ) {
        return false;
    }

    float combinedRadius = radiusA + radiusB;

    // Ignore box radius.. do they still overlap?
    if( GetManifold( boxA, boxB, result ) ) {
        result.penetration += combinedRadius;
        return true;
    }

    Vec2 sideStartsA[4];
    Vec2 sideEndsA[4];

    Vec2 sideStartsB[4];
    Vec2 sideEndsB[4];

    boxA.GetSides( sideStartsA, sideEndsA );
    boxB.GetSides( sideStartsB, sideEndsB );

    Vec2 cornersOfA[4];
    Vec2 cornersOfB[4];

    boxA.GetCorners( cornersOfA[0], cornersOfA[1], cornersOfA[2], cornersOfA[3] );
    boxB.GetCorners( cornersOfB[0], cornersOfB[1], cornersOfB[2], cornersOfB[3] );

    Vec2 bestA;
    Vec2 bestB;

    Vec2 alignedA;
    Vec2 alignedB;

    bool isAligned = false;

    float bestMatchSquared = 9999999.f;
    float epsilon = 0.00001f;

    for( int sideIndex = 0; sideIndex < 4; sideIndex++ ) {
        const Vec2& startA = sideStartsA[sideIndex];
        const Vec2& endA = sideEndsA[sideIndex];

        const Vec2& startB = sideStartsB[sideIndex];
        const Vec2& endB = sideEndsB[sideIndex];

        for( int cornerIndex = 0; cornerIndex < 4; cornerIndex++ ) {
            // Doing two comparisons per iter... sideA to cornerB AND sideB to cornerA
            // Note.. separate IF statements below
            const Vec2& cornerA = cornersOfA[cornerIndex];
            const Vec2& cornerB = cornersOfB[cornerIndex];

            Vec2 closestOnA = GetClosestPointOnLineSegment( cornerB, startA, endA );
            Vec2 closestOnB = GetClosestPointOnLineSegment( cornerA, startB, endB );

            float distanceSquaredA = (closestOnA - cornerB).GetLengthSquared();
            float distanceSquaredB = (closestOnB - cornerA).GetLengthSquared();

            float differenceA = abs(bestMatchSquared - distanceSquaredA);
            float differenceB = abs(bestMatchSquared - distanceSquaredB);

            if( (distanceSquaredA < bestMatchSquared) && (differenceA > epsilon) ) {
                bestA = closestOnA;
                bestB = cornerB;
                bestMatchSquared = distanceSquaredA;
                isAligned = false;
            } else if( differenceA < epsilon ) {
                isAligned = true;
                alignedA = closestOnA;
                alignedB = cornerB;
            }

            if( (distanceSquaredB < bestMatchSquared) && (differenceB > epsilon) ) {
                bestA = cornerA;
                bestB = closestOnB;
                bestMatchSquared = distanceSquaredB;
                isAligned = false;
            } else if( differenceB < epsilon ) {
                isAligned = true;
                alignedA = cornerA;
                alignedB = closestOnB;
            }
        }
    }

    if( bestMatchSquared >= combinedRadius * combinedRadius ) {
        return false;
    }

    // Normal
    result.normal = (bestA - bestB).GetNormalized();

    // Contact Point
    if( isAligned ) {
        Vec2 avgerageB = (bestB + alignedB) * 0.5f;
        result.contactPos = avgerageB + (radiusB * result.normal);
    } else {
        result.contactPos = bestB + (radiusB * result.normal);
    }

    // Penetration
    result.penetration = combinedRadius - sqrt( bestMatchSquared );
    return true;
}


/*
bool Manifold2::GetManifold( const AABB2& boundsA, const AABB2& boundsB, Manifold2& result ) {
    float overlapMinX = std::max( boundsA.mins.x, boundsB.mins.x );
    float overlapMaxX = std::min( boundsA.maxs.x, boundsB.maxs.x );
    float overlapMinY = std::max( boundsA.mins.y, boundsB.mins.y );
    float overlapMaxY = std::min( boundsA.maxs.y, boundsB.maxs.y );

    bool isOverlappingX = overlapMinX < overlapMaxX;
    bool isOverlappingY = overlapMinY < overlapMaxY;

    // They are actually overlapping
    if( isOverlappingX && isOverlappingY ) {
        Vec2 direction = boundsA.GetCenter() - boundsB.GetCenter();

        // Distances it takes to push out of
        float pushLeft  = std::abs( boundsB.mins.x - boundsA.maxs.x );
        float pushRight = std::abs( boundsB.maxs.x - boundsA.mins.x );
        float pushUp    = std::abs( boundsB.maxs.y - boundsA.mins.y );
        float pushDown  = std::abs( boundsB.mins.y - boundsA.maxs.y );
        float distances[4] = { pushLeft, pushRight, pushUp, pushDown };

        float minDistance = 99999999.f;
        int minIndex = -1;

        // Find shortest distance
        for( int dirIndex = 0; dirIndex < 4; dirIndex++ ) {
            float distance = distances[dirIndex];

            if( distance < minDistance ) {
                minDistance = distance;
                minIndex = dirIndex;
            }
        }

        // Set actual result based on minimum direction
        if( minIndex == 0 ) { // Left
            result.normal = Vec2( -1.f,  0.f );
            result.penetration = pushLeft;
        } else if( minIndex == 1 ) { // Right
            result.normal = Vec2(  1.f,  0.f );
            result.penetration = pushRight;
        } else if( minIndex == 2 ) { // Up
            result.normal = Vec2(  0.f,  1.f );
            result.penetration = pushUp;
        } else if( minIndex == 3 ) { // Down
            result.normal = Vec2(  0.f, -1.f );
            result.penetration = pushDown;
        }

        return true;
    }

    return false;
}


bool Manifold2::GetManifold( const AABB2& boundsA, const Vec2& discCenterB, float discRadiusB, Manifold2& result ) {
    bool discInsideBox = boundsA.IsPointInside( discCenterB );
    Vec2 displacement;

    if (discInsideBox ) {
        Vec2 closestPoint = boundsA.GetClosestPointOnAABB2Edge( discCenterB );
        displacement = discCenterB - closestPoint; // Direction reversed due to being on the wrong side of the box edge

        result.penetration = discRadiusB + displacement.GetLength(); // Must also move extra (add the displacement) instead of subtract
    } else {
        Vec2 closestPoint = boundsA.GetClosestPointOnAABB2( discCenterB );
        displacement = closestPoint - discCenterB;
        float radiusSquared = discRadiusB * discRadiusB;

        if( displacement.GetLengthSquared() >= radiusSquared ) {
            // No collision
            result.normal = Vec2::ZERO;
            result.penetration = 0.f;
            return false;
        }
        
        result.penetration = discRadiusB - displacement.GetLength();
    }

    displacement.SetLength( 1.f );
    result.normal = displacement;

    return true;
}


bool Manifold2::GetManifold( const Vec2& discCenterA, float discRadiusA, const Vec2& discCenterB, float discRadiusB, Manifold2& result ) {
    Vec2 displacement = discCenterA - discCenterB;
    float radiiSum = discRadiusA + discRadiusB;
    float radiiSquared = radiiSum * radiiSum;

    if( displacement.GetLengthSquared() >= radiiSquared ) {
        // No collision
        result.normal = Vec2::ZERO;
        result.penetration = 0.f;
        return false;
    }

    result.penetration = radiiSum - displacement.GetLength();

    displacement.SetLength( 1.f );
    result.normal = displacement;

    return true;
}
*/
