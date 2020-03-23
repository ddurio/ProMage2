#include "Engine/Renderer/CPUMesh.hpp"

#include "Engine/Debug/DebugDraw.hpp"
#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Capsule3.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/OBB3.hpp"

#include "ThirdParty/MikkT/mikktspace.h"

#include "iostream"
#include "fstream"
#include "list"


void CPUMesh::Clear() {
    m_verts.clear();
    m_indices.clear();
    m_stamp = VertexMaster();
}


void CPUMesh::SetColor( const Rgba& color ) {
    m_stamp.color = color;
}


void CPUMesh::SetUV( const Vec2& uv ) {
    m_stamp.uvTexCoords = uv;
}


int CPUMesh::AddVertex( const VertexMaster& vertex ) {
    m_verts.push_back( vertex );
    return ((int)m_verts.size() - 1);
}


int CPUMesh::AddVertex( const Vec3& position, const Vec3& normal /*= Vec3( 0.f, 0.f, -1.f ) */ ) {
    m_stamp.position = position;
    m_stamp.normal = normal.GetNormalized();

    if( m_stamp.normal != Vec3::UP && m_stamp.normal != -Vec3::UP ) {
        m_stamp.tangent = CrossProduct( m_stamp.normal, Vec3::UP );
        m_stamp.bitangent = CrossProduct( m_stamp.tangent, m_stamp.normal );
    } else {
        m_stamp.bitangent = CrossProduct( Vec3::SCREEN, m_stamp.normal );
        m_stamp.tangent = CrossProduct( m_stamp.normal, m_stamp.bitangent );
    }

    m_verts.push_back( m_stamp );
    return ((int)m_verts.size() - 1);
}

void CPUMesh::AddVertexArray( const VertexList& verts ) {
    int numVerts = (int)verts.size();

    for( int triangleIndex = 0; (3 * triangleIndex) + 2 < numVerts; triangleIndex++ ) {
        // Get index into array
        int arrayIndex0 = 3 * triangleIndex;
        int arrayIndex1 = arrayIndex0 + 1;
        int arrayIndex2 = arrayIndex0 + 2;

        // Get verts
        const VertexPCU& vert0 = verts[arrayIndex0];
        const VertexPCU& vert1 = verts[arrayIndex1];
        const VertexPCU& vert2 = verts[arrayIndex2];

        // Add verts
        SetColor( vert0.color );
        SetUV( vert0.uvTexCoords );
        int vertIndex0 = AddVertex( vert0.position );

        SetColor( vert1.color );
        SetUV( vert1.uvTexCoords );
        int vertIndex1 = AddVertex( vert1.position );

        SetColor( vert2.color );
        SetUV( vert2.uvTexCoords );
        int vertIndex2 = AddVertex( vert2.position );

        // Add indexes
        AddIndexedTriangle( vertIndex0, vertIndex1, vertIndex2 );
    }
}


void CPUMesh::SetTangent( const Vec3& tangent ) {
    m_stamp.tangent = tangent;
}


void CPUMesh::SetBitangent( const Vec3& bitangent ) {
    m_stamp.bitangent = bitangent;
}


void CPUMesh::SetNormal( const Vec3& normal ) {
    m_stamp.normal = normal;
}


void CPUMesh::SetDefaultMaterial( const std::string& materialName ) {
    m_defaultMaterial = materialName;
}


std::string CPUMesh::GetDefaultMaterial() const {
    return m_defaultMaterial;
}


IntVec3 CPUMesh::AddTriangle( const Vec3& position0, const Vec3& position1, const Vec3& position2 ) {
    int index0 = AddVertex( position0 );
    int index1 = AddVertex( position1 );
    int index2 = AddVertex( position2 );

    AddIndexedTriangle( index0, index1, index2 );
    return IntVec3( index0, index1, index2 );
}


IntVec3 CPUMesh::AddTriangle( const Vec2& position0, const Vec2& position1, const Vec2& position2 ) {
    return AddTriangle( Vec3( position0, 0.f ), Vec3( position1, 0.f ), Vec3( position2, 0.f ) );
}


void CPUMesh::AddQuad( const AABB2& quad, const AABB2& uvs /*= AABB2::UVZEROTOONE */ ) {
    Vec3 mins = Vec3( quad.mins.x, quad.mins.y, 0.f );
    Vec3 maxs = Vec3( quad.maxs.x, quad.maxs.y, 0.f );
    AddQuad( mins, maxs, uvs, COORD_PLANE_XY );
}


void CPUMesh::AddQuad( const OBB2& box, const AABB2& uvs /*= AABB2::UVZEROTOONE */ ) {
    Vec2 positionTL2;
    Vec2 positionTR2;
    Vec2 positionBL2;
    Vec2 positionBR2;

    box.GetCorners( positionTL2, positionTR2, positionBL2, positionBR2 );

    Vec3 positionTL = Vec3( positionTL2, 0.f );
    Vec3 positionTR = Vec3( positionTR2, 0.f );
    Vec3 positionBL = Vec3( positionBL2, 0.f );
    Vec3 positionBR = Vec3( positionBR2, 0.f );

    AddQuad( positionTL, positionTR, positionBL, positionBR, uvs );
}


void CPUMesh::AddQuad( const Vec2& mins, const Vec2& maxs, const AABB2& uvs /*= AABB2::UVZEROTOONE*/, CoordinatePlane plane /*= COORD_PLANE_XY */ ) {
    AddQuad( Vec3( mins, 0.f ), Vec3( maxs, 0.f ), uvs, plane );
}


void CPUMesh::AddQuad( const Vec3& mins, const Vec3& maxs, const AABB2& uvs /*= AABB2::UVZEROTOONE*/, CoordinatePlane plane /*= COORD_PLANE_XY */ ) {
    Vec3 topLeft;
    Vec3 topRight;
    Vec3 botLeft;
    Vec3 botRight;

    switch(plane) {
        case(COORD_PLANE_XY): {
            topLeft  = Vec3( mins.x, maxs.y, mins.z );
            topRight = Vec3( maxs.x, maxs.y, mins.z );
            botLeft  = Vec3( mins.x, mins.y, mins.z );
            botRight = Vec3( maxs.x, mins.y, mins.z );
            break;
        } case(COORD_PLANE_XZ): {
            topLeft  = Vec3( mins.x, mins.y, maxs.z );
            topRight = Vec3( maxs.x, mins.y, maxs.z );
            botLeft  = Vec3( mins.x, mins.y, mins.z );
            botRight = Vec3( maxs.x, mins.y, mins.z );
            break;
        } case(COORD_PLANE_ZY): {
            topLeft  = Vec3( mins.x, maxs.y, mins.z );
            topRight = Vec3( mins.x, maxs.y, maxs.z );
            botLeft  = Vec3( mins.x, mins.y, mins.z );
            botRight = Vec3( mins.x, mins.y, maxs.z );
            break;
        }
    }

    AddQuad( topLeft, topRight, botLeft, botRight, uvs );
}


void CPUMesh::AddQuad( const Vec2& positionTL, const Vec2& positionTR, const Vec2& positionBL, const Vec2& positionBR, const AABB2& uvs /*= AABB2::UVZEROTOONE */ ) {
    AddQuad( Vec3( positionTL, 0.f ), Vec3( positionTR, 0.f ), Vec3( positionBL, 0.f ), Vec3( positionBR, 0.f ), uvs );
}


// UVs are assuming image is facing to the RIGHT
void CPUMesh::AddQuad( const Vec3& positionTL, const Vec3& positionTR, const Vec3& positionBL, const Vec3& positionBR, const AABB2& uvs /*= AABB2::UVZEROTOONE */ ) {
    Vec3 normal = CrossProduct( positionTR - positionBR, positionTR - positionTL );

    //SetUV( Vec2::ZERO );
    SetUV( Vec2( uvs.mins.x, uvs.maxs.y ) );
    int indexTL = AddVertex( positionTL, normal );

    //SetUV( Vec2( 1.f, 0.f ) );
    SetUV( uvs.maxs );
    int indexTR = AddVertex( positionTR, normal );

    //SetUV( Vec2( 0.f, 1.f ) );
    SetUV( uvs.mins );
    int indexBL = AddVertex( positionBL, normal );

    //SetUV( Vec2::ONE );
    SetUV( Vec2( uvs.maxs.x, uvs.mins.y ) );
    int indexBR = AddVertex( positionBR, normal );

    AddIndexedQuad( indexTL, indexTR, indexBL, indexBR );
}


void CPUMesh::AddQuadEdge( OBB2 box, float thickness ) {
    box.halfExtents -= Vec2( 0.5f * thickness ); // Makes outside edge flush with quad edge

    Vec2 corners[4];
    box.GetCorners( corners );

    AddLine( corners[0], corners[1], thickness ); // Top
    AddLine( corners[2], corners[3], thickness ); // Bottom
    AddLine( corners[0], corners[2], thickness ); // Left
    AddLine( corners[1], corners[3], thickness ); // Right
}


void CPUMesh::AddRoundedQuad( const OBB2& box, float radius, const AABB2& uvs /*= AABB2::UVZEROTOONE*/ ) {
    if( radius == 0.f ) {
        AddQuad( box, uvs );
        return;
    } else if( box.halfExtents == Vec2::ZERO ) {
        AddCircle( box.center, radius ); // FIXME: This ignores the uvs.. oops
        return;
    } else if( box.halfExtents.x == 0.f || box.halfExtents.y == 0 ) {
        Vec2 corners[4];
        box.GetCorners( corners );

        Vec2 start = corners[0];
        Vec2 end;

        if( box.halfExtents.x == 0.f ) {
            end = corners[2];
        } else {
            end = corners[1];
        }

        Capsule2 cap2 = Capsule2( start, end, radius );
        AddCapsule( cap2 ); // FIXME: This ignores the uvs.. oops
        return;
    }

    /* Rounded OBB Subdivision
                @@@----------@@@      -
              @@@ |          | @@@    |
             @@ D |          | D @@   |
        -    @----|          |----@   |
        |    |    |          |    |   |
   2*Ey |    | B2 |    B1    | B3 |   | 2*Ey + 2*r
        |    |    |          |    |   |
        |    |    |          |    |   |
        -    @----|          |----@   |
             @@ D |          | D @@   |
              @@@ |          | @@@    |
                @@@----------@@@      -

                  |---2*Ex---|
    */

    float uvRatioX = (uvs.maxs.x - uvs.mins.x) / (2.f * box.halfExtents.x);
    float uvRatioY = (uvs.mins.y - uvs.maxs.y) / (2.f * box.halfExtents.y);
    Vec2 uvRatio = Vec2( uvRatioX, uvRatioY );
    Vec2 uvOffset = uvRatio * radius;

    // B1
    OBB2 box1Bounds = box;
    box1Bounds.halfExtents.y += radius;

    AABB2 box1UVs = uvs;
    box1UVs.mins.y += uvOffset.y;
    box1UVs.maxs.y -= uvOffset.y;

    AddQuad( box1Bounds, box1UVs );

    // B2 / B3
    float halfRadius = 0.5f * radius;
    float sideBoxDistFromCenter = box.halfExtents.x + halfRadius;
    Vec2 sideBoxOffsetFromCenter = box.right * sideBoxDistFromCenter;

    Vec2 sideBoxHalfExtents = Vec2( halfRadius, box.halfExtents.y );

    // B2
    Vec2 box2Center = box.center - sideBoxOffsetFromCenter;
    OBB2 box2Bounds = OBB2( box2Center, sideBoxHalfExtents, box.right );

    AABB2 box2UVs = uvs;
    box2UVs.mins.x -= uvOffset.x;
    box2UVs.maxs.x = uvs.mins.x;

    AddQuad( box2Bounds, box2UVs );

    // B3
    Vec2 box3Center = box.center + sideBoxOffsetFromCenter;
    OBB2 box3Bounds = OBB2( box3Center, sideBoxHalfExtents, box.right );

    AABB2 box3UVs = uvs;
    box3UVs.mins.x = uvs.maxs.x;
    box3UVs.maxs.x += uvOffset.x;

    AddQuad( box3Bounds, box3UVs );

    // Corners
    Vec2 corners[4];
    box.GetCorners( corners[0], corners[1], corners[2], corners[3] );
    float rightDegrees  = box.right.GetAngleDegrees();
    float upDegrees     = rightDegrees  + 90.f;
    float leftDegrees   = upDegrees     + 90.f;
    float downDegrees   = leftDegrees   + 90.f;

    AddPartialRing( corners[0], 0.f, radius, upDegrees, leftDegrees    ); // TL - 0
    AddPartialRing( corners[1], 0.f, radius, rightDegrees, upDegrees   ); // TR - 1
    AddPartialRing( corners[2], 0.f, radius, leftDegrees, downDegrees  ); // BL - 2
    AddPartialRing( corners[3], 0.f, radius, downDegrees, rightDegrees + 360.f ); // BR - 3
}


void CPUMesh::AddRoundedQuadEdge( const OBB2& box, float radius, float thickness ) {
    if( radius == 0.f ) {
        AddQuadEdge( box, thickness );
        return;
    } else if( box.halfExtents == Vec2::ZERO ) {
        //AddRing( box.center, radius );
        float innerRadius = radius - (0.5f * thickness);
        AddRing( box.center, innerRadius, radius );
        return;
    } else if( box.halfExtents.x == 0.f || box.halfExtents.y == 0 ) {
        Vec2 corners[4];
        box.GetCorners( corners );

        Vec2 start = corners[0];
        Vec2 end;

        if( box.halfExtents.x == 0.f ) {
            end = corners[2];
        } else {
            end = corners[1];
        }

        Capsule2 cap2 = Capsule2( start, end, radius );
        AddCapsuleEdge( cap2, thickness );
        return;
    }

    /* Rounded OBB Subdivision
                @@@----------@@@      -
              @@@ |          | @@@    |
             @@ D |          | D @@   |
        -    @----|          |----@   |
        |    |    |          |    |   |
   2*Ey |    | B2 |    B1    | B3 |   | 2*Ey + 2*r
        |    |    |          |    |   |
        |    |    |          |    |   |
        -    @----|          |----@   |
             @@ D |          | D @@   |
              @@@ |          | @@@    |
                @@@----------@@@      -

                  |---2*Ex---|
    */

    // B1
    float halfThickness = 0.5f * thickness;
    OBB2 box1Bounds = box;
    box1Bounds.halfExtents.y += (radius - halfThickness);

    Vec2 corners[4];
    box1Bounds.GetCorners( corners );

    AddLine( corners[0], corners[1], thickness );
    AddLine( corners[2], corners[3], thickness );

    // B2 / B3
    float halfRadius = 0.5f * radius;
    float sideBoxDistFromCenter = box.halfExtents.x + halfRadius;
    Vec2 sideBoxOffsetFromCenter = box.right * sideBoxDistFromCenter;

    Vec2 sideBoxHalfExtents = Vec2( halfRadius, box.halfExtents.y ) - Vec2( halfThickness );

    // B2
    Vec2 box2Center = box.center - sideBoxOffsetFromCenter;
    OBB2 box2Bounds = OBB2( box2Center, sideBoxHalfExtents, box.right );

    box2Bounds.GetCorners( corners );
    AddLine( corners[0], corners[2], thickness );

    // B3
    Vec2 box3Center = box.center + sideBoxOffsetFromCenter;
    OBB2 box3Bounds = OBB2( box3Center, sideBoxHalfExtents, box.right );

    box3Bounds.GetCorners( corners );
    AddLine( corners[1], corners[3], thickness );

    // Corners
    box.GetCorners( corners[0], corners[1], corners[2], corners[3] );
    float rightDegrees  = box.right.GetAngleDegrees();
    float upDegrees     = rightDegrees  + 90.f;
    float leftDegrees   = upDegrees     + 90.f;
    float downDegrees   = leftDegrees   + 90.f;

    float innerRadius = radius - thickness;

    AddPartialRing( corners[0], innerRadius, radius, upDegrees, leftDegrees    ); // TL - 0
    AddPartialRing( corners[1], innerRadius, radius, rightDegrees, upDegrees   ); // TR - 1
    AddPartialRing( corners[2], innerRadius, radius, leftDegrees, downDegrees  ); // BL - 2
    AddPartialRing( corners[3], innerRadius, radius, downDegrees, rightDegrees + 360.f ); // BR - 3
}


void CPUMesh::AddBox( const Vec3& mins, const Vec3& maxs ) {
    Vec3 faceMins;
    Vec3 faceMaxs;

    // Front
    faceMins = mins;
    faceMaxs = Vec3( maxs.x, maxs.y, mins.z );
    AddQuad( faceMins, faceMaxs );

    // Back
    faceMins = Vec3( maxs.x, mins.y, maxs.z );
    faceMaxs = Vec3( mins.x, maxs.y, maxs.z );
    AddQuad( faceMins, faceMaxs );

    // Left
    faceMins = Vec3( mins.x, mins.y, maxs.z );
    faceMaxs = Vec3( mins.x, maxs.y, mins.z );
    AddQuad( faceMins, faceMaxs, AABB2::UVZEROTOONE, COORD_PLANE_ZY );

    // Right
    faceMins = Vec3( maxs.x, mins.y, mins.z );
    faceMaxs = maxs;
    AddQuad( faceMins, faceMaxs, AABB2::UVZEROTOONE, COORD_PLANE_ZY );

    // Top
    faceMins = Vec3( mins.x, maxs.y, mins.z );
    faceMaxs = maxs;
    AddQuad( faceMins, faceMaxs, AABB2::UVZEROTOONE, COORD_PLANE_XZ );

    // Bottom
    faceMins = Vec3( mins.x, mins.y, maxs.z );
    faceMaxs = Vec3( maxs.x, mins.y, mins.z );
    AddQuad( faceMins, faceMaxs, AABB2::UVZEROTOONE, COORD_PLANE_XZ );
}


void CPUMesh::AddBox( const OBB3& box ) {
    Vec3 forward = CrossProduct( box.right, box.up );

    Vec3 posFTL = box.center - (box.halfExtents.z * forward) + (box.halfExtents.y * box.up) - (box.halfExtents.x * box.right);
    Vec3 posFTR = box.center - (box.halfExtents.z * forward) + (box.halfExtents.y * box.up) + (box.halfExtents.x * box.right);
    Vec3 posFBL = box.center - (box.halfExtents.z * forward) - (box.halfExtents.y * box.up) - (box.halfExtents.x * box.right);
    Vec3 posFBR = box.center - (box.halfExtents.z * forward) - (box.halfExtents.y * box.up) + (box.halfExtents.x * box.right);

    Vec3 posBTL = box.center + (box.halfExtents.z * forward) + (box.halfExtents.y * box.up) - (box.halfExtents.x * box.right);
    Vec3 posBTR = box.center + (box.halfExtents.z * forward) + (box.halfExtents.y * box.up) + (box.halfExtents.x * box.right);
    Vec3 posBBL = box.center + (box.halfExtents.z * forward) - (box.halfExtents.y * box.up) - (box.halfExtents.x * box.right);
    Vec3 posBBR = box.center + (box.halfExtents.z * forward) - (box.halfExtents.y * box.up) + (box.halfExtents.x * box.right);

    AddQuad( posFTL, posFTR, posFBL, posFBR ); // Front
    AddQuad( posBTR, posBTL, posBBR, posBBL ); // Back
    AddQuad( posBTL, posFTL, posBBL, posFBL ); // Left
    AddQuad( posFTR, posBTR, posFBR, posBBR ); // Right
    AddQuad( posBTL, posBTR, posFTL, posFTR ); // Top
    AddQuad( posFBL, posFBR, posBBL, posBBR ); // Bottom
}


void CPUMesh::AddCapsule( const Capsule2& capsule ) {
    AddCircle( capsule.start, capsule.radius );
    AddCircle( capsule.end, capsule.radius );

    Vec2 bodyCenter = (capsule.start + capsule.end) * 0.5f;
    Vec2 bodyRight = capsule.end - capsule.start;
    Vec2 bodyDimensions = Vec2( 0.5f * bodyRight.GetLength(), capsule.radius );

    OBB2 body = OBB2( bodyCenter, bodyDimensions, bodyRight );
    AddQuad( body );
}

void CPUMesh::AddCapsule( const Capsule3& capsule ) {
    AddUVSphere( capsule.start, capsule.radius );
    AddUVSphere( capsule.end, capsule.radius );

    Vec3 normal = capsule.end - capsule.start;
    float height = normal.NormalizeGetPreviousLength();
    AddCylinder( capsule.GetCenter(), height, capsule.radius, normal );
}


void CPUMesh::AddCapsuleEdge( const Capsule2& capsule, float thickness ) {
    Vec2 startToEnd = capsule.end - capsule.start;
    Vec2 tangent = startToEnd.GetRotated90Degrees();
    float startDegrees = tangent.GetAngleDegrees();

    float innerRadius = capsule.radius - thickness;
    float outerRadius = capsule.radius;

    AddPartialRing( capsule.start, innerRadius, outerRadius, startDegrees, startDegrees + 180.f );
    AddPartialRing( capsule.end, innerRadius, outerRadius, startDegrees + 180.f, startDegrees + 360.f );

    Vec2 bodyCenter = capsule.GetCenter();
    Vec2 bodyRight = startToEnd;
    Vec2 bodyDimensions = Vec2( 0.5f * bodyRight.GetLength(), capsule.radius ) - Vec2( 0.5f * thickness );

    OBB2 body = OBB2( bodyCenter, bodyDimensions, bodyRight );
    Vec2 corners[4];
    body.GetCorners( corners[0], corners[1], corners[2], corners[3] );

    AddLine( corners[0], corners[1], thickness );
    AddLine( corners[2], corners[3], thickness );
}


/*
void CPUMesh::AddCapsuleEdge( const Capsule3& capsule, float thickness ) {

}
*/


void CPUMesh::AddLine( const Vec2& start, const Vec2& end, float thickness ) {
    Vec3 start3 = Vec3( start, 0.f );
    Vec3 end3 = Vec3( end, 0.f );
    AddLine( start3, end3, thickness );
}


void CPUMesh::AddLine( const Vec3& start, const Vec3& end, float thickness ) {
    Vec3 center = (start + end) * 0.5f;
    Vec3 forward = end - start;

    float length = forward.NormalizeGetPreviousLength();
    float halfThickness = 0.5f * thickness;
    Vec3 halfExtents = Vec3( halfThickness, halfThickness, (0.5f * length) + halfThickness );

    Vec3 right = CrossProduct( Vec3::UP, forward ).GetNormalized();

    if( right == Vec3::ZERO ) {
        right = CrossProduct( Vec3::RIGHT, forward ).GetNormalized();
    }

    Vec3 up = CrossProduct( forward, right ).GetNormalized();

    OBB3 box = OBB3( center, halfExtents, right, up );
    AddBox( box );
}


void CPUMesh::AddArrow( const Vec2& start, const Vec2& end, float thickness ) {
    // Make line
    Vec2 displacement = end - start;
    Vec2 lineEnd = start + displacement * 0.8f;
    AddLine( start, lineEnd, thickness );

    // Make arrow head
    Vec2 lineLeft = displacement.GetNormalized().GetRotated90Degrees();
    Vec2 offsetLeft = 2.f * thickness * lineLeft;

    Vec2 leftArrowPoint = lineEnd + offsetLeft;
    Vec2 rightArrowPoint = lineEnd - offsetLeft;

    AddTriangle( leftArrowPoint, end, rightArrowPoint );
}


void CPUMesh::AddCone( const Vec3& position, float height, float radius, const Vec3& upVector /*= Vec3( 0, 1, 0)*/, int numWedges /*= 32*/ ) {
    Vec3 normal = upVector.GetNormalized();
    Vec3 cylinderPos = position + (height * 0.5f) * normal;
    AddNonUniformCylinder( cylinderPos, height, radius, 0.f, normal, numWedges );
}


void CPUMesh::AddHourGlass( const Vec3& position, float height, float radius, const Vec3& normal /*= Vec3( 0, 1, 0)*/, int numWedges /*= 32*/ ) {
    AddNonUniformCylinder( position, height, radius, -radius, normal, numWedges );
}



void CPUMesh::AddCylinder( const Vec3& position, float height, float radius, const Vec3& normal /*= Vec3( 0, 1, 0)*/, int numWedges /*= 32*/ ) {
    AddNonUniformCylinder( position, height, radius, radius, normal, numWedges );
}


void CPUMesh::AddNonUniformCylinder( const Vec3& position, float height, float radiusBot, float radiusTop, const Vec3& upVector /*= Vec3( 0, 1, 0)*/, int numWedges /*= 32*/ ) {
    Vec3 normal = upVector.GetNormalized();
    Vec3 posTopCenter = position + (height * 0.5f) * normal;
    Vec3 posBotCenter = position - (height * 0.5f) * normal;

    Vec3 posTL = Vec3::ZERO;
    Vec3 posTR = Vec3::ZERO;
    Vec3 posBL = Vec3::ZERO;
    Vec3 posBR = Vec3::ZERO;

    int indexTL = 0;
    int indexTR = 0;
    int indexBL = 0;
    int indexBR = 0;

    int indexFirstTop = 0;
    int indexFirstBot = 0;
    int indexTopCenter = -1;
    int indexBotCenter = -1;

    if( radiusTop == 0 ) {
        indexTopCenter = AddVertex( posTopCenter, normal );
    } else {
        AddCircle( posTopCenter, radiusTop, normal, numWedges );
    }

    if( radiusBot == 0 ) {
        indexBotCenter = AddVertex( posBotCenter, -normal );
    } else {
        AddCircle( posBotCenter, radiusBot, -normal, numWedges );
    }

    float degreesPerWedge = 360.f / numWedges;
    Matrix44 rotationTop = Matrix44::MakeRotationBetweenVectors( Vec3::UP, normal );
    Matrix44 rotationBot = rotationTop;
    rotationTop.SetTranslation( posTopCenter );
    rotationBot.SetTranslation( posBotCenter );

    for( int wedgeIndex = 0; wedgeIndex < numWedges; wedgeIndex++ ) {
        if( wedgeIndex == 0 ) {
            // First iter... setup initial points

            // Top
            if( radiusTop == 0.f ) {
                indexTL = indexTopCenter;
                indexTR = indexTopCenter;
            } else {
                // Position around unit circle
                posTL = TransformPosition( Vec3( 1.f, 0.f, 0.f ), radiusTop, wedgeIndex * degreesPerWedge, Vec3::ZERO );
                posTL = Vec3( posTL.x, 0.f, posTL.y );

                // Rotate & Translate unit circle 
                posTL = rotationTop.TransformPosition3D( posTL );
                indexTL = AddVertex( posTL, posTL - posTopCenter );
            }

            // Bottom
            if( radiusBot == 0.f ) {
                indexBL = indexBotCenter;
                indexBR = indexBotCenter;
            } else {
                // Position around unit circle
                posBL = TransformPosition( Vec3( 1.f, 0.f, 0.f ), radiusBot, wedgeIndex * degreesPerWedge, Vec3::ZERO );
                posBL = Vec3( posBL.x, 0.f, posBL.y );

                // Rotate & Translate unit circle 
                posBL = rotationBot.TransformPosition3D( posBL );
                indexBL = AddVertex( posBL, posBL - posBotCenter );
            }

            indexFirstTop = indexTL;
            indexFirstBot = indexBL;
        } else if( wedgeIndex == numWedges - 1 ) {
            // Last iter... all verts already added
            indexTR = indexFirstTop;
            indexBR = indexFirstBot;

            // Add triangles
            AddIndexedQuad( indexTL, indexTR, indexBL, indexBR );   // Add outer face

            continue; // Equivalent to a break
        }

        if( radiusTop != 0.f ) {
            // Position around unit circle
            posTR = TransformPosition( Vec3( 1.f, 0.f, 0.f ), radiusTop, (wedgeIndex + 1) * degreesPerWedge, Vec3::ZERO );
            posTR = Vec3( posTR.x, 0.f, posTR.y );

            // Rotate & Translate unit circle 
            posTR = rotationTop.TransformPosition3D( posTR );
            indexTR = AddVertex( posTR, posTR - posTopCenter );
        }

        if( radiusBot != 0.f ) {
            posBR = TransformPosition( Vec3( 1.f, 0.f, 0.f ), radiusBot, (wedgeIndex + 1) * degreesPerWedge, Vec3::ZERO );
            posBR = Vec3( posBR.x, 0.f, posBR.y );

            // Rotate & Translate unit circle 
            posBR = rotationBot.TransformPosition3D( posBR );
            indexBR = AddVertex( posBR, posBR - posBotCenter );
        }

        // Add triangles
        AddIndexedQuad( indexTL, indexTR, indexBL, indexBR );   // Add outer face

        indexTL = indexTR;
        indexBL = indexBR;
    }
}


void CPUMesh::AddCircle( const Vec2& center, float radius, int numWedges /*= 32 */ ) {
    Vec3 center3 = Vec3( center, 0.f );
    Vec3 forward = Vec3::SCREEN;
    AddCircle( center3, radius, forward, numWedges );
}


void CPUMesh::AddCircle( const Vec3& center, float radius, const Vec3& forwardVector /*= Vec3::SCREEN*/, int numWedges /*= 32*/ ) {
    Vec3 normal = forwardVector.GetNormalized();

    Vec3 posOuterRight = Vec3::ZERO;
    Vec3 posOuterLeft = Vec3::ZERO;

    int indexOuterLeft = 0;
    int indexOuterRight = 0;

    int indexFirst = 0;
    int indexCenter = AddVertex( center, normal );

    float degreesPerWedge = 360.f / numWedges;
    Matrix44 rotation = Matrix44::MakeRotationBetweenVectors( Vec3::UP, normal );
    rotation.SetTranslation( center );

    for( int wedgeIndex = 0; wedgeIndex < numWedges; wedgeIndex++ ) {
        if( wedgeIndex == 0 ) {
            // Position around unit circle
            posOuterLeft = TransformPosition( Vec3( 1.f, 0.f, 0.f ), radius, wedgeIndex * degreesPerWedge, Vec3::ZERO );
            posOuterRight = Vec3( posOuterRight.x, 0.f, posOuterRight.y );

            // Rotate & Translate unit circle 
            posOuterLeft = rotation.TransformPosition3D( posOuterLeft );
            indexOuterRight = AddVertex( posOuterLeft, normal );

            indexFirst = indexOuterRight;
        } else if( wedgeIndex == numWedges - 1 ) {
            // Last iter... all verts already added
            indexOuterLeft = indexFirst;
            AddIndexedTriangle( indexCenter, indexOuterLeft, indexOuterRight );
            continue;
        }

        // Position around unit circle
        posOuterRight = TransformPosition( Vec3( 1.f, 0.f, 0.f ), radius, (wedgeIndex + 1) * degreesPerWedge, Vec3::ZERO );
        posOuterRight = Vec3( posOuterRight.x, 0.f, posOuterRight.y );

        // Rotate & Translate unit circle 
        posOuterRight = rotation.TransformPosition3D( posOuterRight );

        // Add Verts and Triangle
        indexOuterLeft = AddVertex( posOuterRight, normal );
        AddIndexedTriangle( indexCenter, indexOuterLeft, indexOuterRight );
        indexOuterRight = indexOuterLeft;
    }
}


void CPUMesh::AddRing( const Vec2& center, float radiusInner, float radiusOuter, int numWedges /*= 32 */ ) {
    AddPartialRing( center, radiusInner, radiusOuter, 0.f, 360.f, numWedges );
}


void CPUMesh::AddPartialRing( const Vec2& center, float radiusInner, float radiusOuter, float degreesStart, float degreesEnd, int numWedgesForCircle /*= 32 */ ) {
    Vec2 posInnerLeft  = Vec2::ZERO;
    Vec2 posInnerRight = Vec2::ZERO;
    Vec2 posOuterLeft  = Vec2::ZERO;
    Vec2 posOuterRight = Vec2::ZERO;

    int indexInnerLeft  = 0;
    int indexInnerRight = 0;
    int indexOuterLeft  = 0;
    int indexOuterRight = 0;

    int indexFirstInner = 0;
    int indexFirstOuter = 0;

    float degreesPerWedge = 360.f / numWedgesForCircle;

    float start = Min( degreesStart, degreesEnd );
    float end = Max( degreesStart, degreesEnd );
    degreesStart = start;
    degreesEnd = end;

    float degreesAround = degreesEnd - degreesStart;
    float numActualWedges = degreesAround / degreesPerWedge;
    int numWedges = RoundToInt( numActualWedges );

    if( numWedges == 0 ) {
        ERROR_RECOVERABLE( "(CPUMesh) No wedges to draw during ring" );
        return;
    }

    degreesPerWedge *= numActualWedges / (float)numWedges;

    for( int wedgeIndex = 0; wedgeIndex < numWedges; wedgeIndex++ ) {
        if( wedgeIndex == 0 ) {
            // Position around unit circle
            posOuterRight = TransformPosition( Vec2( 1.f, 0.f ), radiusOuter, wedgeIndex * degreesPerWedge + degreesStart, Vec2::ZERO );
            posOuterRight = center + posOuterRight;
            indexOuterRight = AddVertex( Vec3( posOuterRight, 0.f ) );

            posInnerRight = TransformPosition( Vec2( 1.f, 0.f ), radiusInner, wedgeIndex * degreesPerWedge + degreesStart, Vec2::ZERO );
            posInnerRight = center + posInnerRight;
            indexInnerRight = AddVertex( Vec3( posInnerRight, 0.f ) );
            
            indexFirstOuter = indexOuterRight;
            indexFirstInner = indexInnerRight;
            /*
        } else if( wedgeIndex == numWedges - 1 ) {
            // Last iter... all verts already added
            indexOuterLeft = indexFirstOuter;
            indexInnerLeft = indexFirstInner;
            AddIndexedQuad( indexOuterLeft, indexOuterRight, indexInnerLeft, indexInnerRight );
            continue;
            */
        }

        // Position around unit circle
        posOuterLeft = TransformPosition( Vec2( 1.f, 0.f ), radiusOuter, (wedgeIndex + 1) * degreesPerWedge + degreesStart, Vec2::ZERO );
        posOuterLeft = center + posOuterLeft;

        posInnerLeft = TransformPosition( Vec2( 1.f, 0.f ), radiusInner, (wedgeIndex + 1) * degreesPerWedge + degreesStart, Vec2::ZERO );
        posInnerLeft = center + posInnerLeft;

        // Add Verts and Triangle
        indexOuterLeft = AddVertex( Vec3( posOuterLeft, 0.f ) );
        indexInnerLeft = AddVertex( Vec3( posInnerLeft, 0.f ) );

        AddIndexedQuad( indexOuterLeft, indexOuterRight, indexInnerLeft, indexInnerRight );
        indexOuterRight = indexOuterLeft;
        indexInnerRight = indexInnerLeft;
    }
}


void CPUMesh::AddUVSphere( const Vec3& center, float radius, int numWedges /*= 32*/, int numSlices /*= 16 */ ) {
    float stepPerWedge = 1.f / numWedges;
    float stepPerSlice = 1.f / numSlices;

    float degreesPerWedge = 360.f * stepPerWedge;
    float degreesPerSlice = 180.f * stepPerSlice;

    std::vector<int> previousSliceIndexes;
    std::vector<int> currentSliceIndexes;

    for( int sliceIndex = 0; sliceIndex <= numSlices; sliceIndex++ ) {
        float sliceAngleTop = sliceIndex * degreesPerSlice;
        float sliceAngleBot = sliceAngleTop + degreesPerSlice;

        float vTop = sliceIndex * stepPerSlice;
        float vBot = vTop + stepPerSlice;

        for( int wedgeIndex = 0; wedgeIndex <= numWedges; wedgeIndex++ ) {
            float wedgeAngleLeft = wedgeIndex * degreesPerWedge;
            float wedgeAngleRight = wedgeAngleLeft + degreesPerWedge;

            float uLeft = wedgeIndex * stepPerWedge;
            float uRight = uLeft + stepPerWedge;

            Vec3 topLeft  = GetCartesianFromSpherical( radius, wedgeAngleLeft,  sliceAngleTop ) + center;
            Vec3 topRight = GetCartesianFromSpherical( radius, wedgeAngleRight, sliceAngleTop ) + center;
            Vec3 botLeft  = GetCartesianFromSpherical( radius, wedgeAngleLeft,  sliceAngleBot ) + center;
            Vec3 botRight = GetCartesianFromSpherical( radius, wedgeAngleRight, sliceAngleBot ) + center;

            if( sliceIndex == 0 ) {
                // Add top cap
                // Because sliceAngleTop == 0, topLeft == topRight == (0, 0, radius)
                int indexTop;

                if( previousSliceIndexes.size() == 0 ) {
                    SetUV( Vec2::ZERO );
                    indexTop = AddVertex( topLeft, topLeft - center );
                    previousSliceIndexes.push_back( indexTop );
                } else {
                    indexTop = previousSliceIndexes[0];
                }

                int indexBL;

                if( currentSliceIndexes.size() == 0 ) {
                    SetUV( Vec2( uLeft, vBot ) );
                    indexBL = AddVertex( botLeft, botLeft - center );
                    currentSliceIndexes.push_back( indexBL );
                } else {
                    indexBL = currentSliceIndexes[wedgeIndex];
                }

                SetUV( Vec2( uRight, vBot ) );
                int indexBR = AddVertex( botRight, botRight - center );
                currentSliceIndexes.push_back( indexBR );

                AddIndexedTriangle( indexTop, indexBR, indexBL );
            } else if( sliceIndex == numSlices ) {
                // Add bottom cap
                // Because sliceAngleBot == 180, botLeft == botRight == (0, 0, -radius)
                int indexTL = previousSliceIndexes[wedgeIndex];
                int indexTR = previousSliceIndexes[wedgeIndex + 1];
                int indexBot;

                if( currentSliceIndexes.size() == 0 ) {
                    SetUV( Vec2::ONE );
                    indexBot = AddVertex( botLeft, botLeft - center );
                    currentSliceIndexes.push_back( indexBot );
                } else {
                    indexBot = currentSliceIndexes[0];
                }

                AddIndexedTriangle( indexTL, indexTR, indexBot );
            } else {
                // Most common case: adding quad somewhere along the face of the sphere
                // Grab two top points from previous indexes
                // 
                int indexTL = previousSliceIndexes[wedgeIndex];
                int indexTR = previousSliceIndexes[wedgeIndex + 1];
                int indexBL;

                if( currentSliceIndexes.size() == 0 ) {
                    SetUV( Vec2( uLeft, vBot ) );
                    indexBL = AddVertex( botLeft, botLeft - center );
                    currentSliceIndexes.push_back( indexBL );
                } else {
                    indexBL = currentSliceIndexes[wedgeIndex];
                }

                SetUV( Vec2( uRight, vBot ) );
                int indexBR = AddVertex( botRight, botRight - center );
                currentSliceIndexes.push_back( indexBR );

                AddIndexedQuad( indexTL, indexTR, indexBL, indexBR );
            }
        }

        previousSliceIndexes = currentSliceIndexes;
        currentSliceIndexes.clear();
    }
}


void CPUMesh::AddIndexedTriangle( int index0, int index1, int index2 ) {
    m_indices.push_back( index0 );
    m_indices.push_back( index1 );
    m_indices.push_back( index2 );
}


void CPUMesh::AddIndexedQuad( int indexTL, int indexTR, int indexBL, int indexBR ) {
    // Clockwise
    AddIndexedTriangle( indexBL, indexTL, indexBR );
    AddIndexedTriangle( indexBR, indexTL, indexTR );

    // Counter
    //AddIndexedTriangle( indexBL, indexBR, indexTL );
    //AddIndexedTriangle( indexBR, indexTR, indexTL );
}


void CPUMesh::LoadObjFile( const std::string& filePath, bool invert, float scale, const std::string& transform ) {
    Matrix44 transformMatrix = Matrix44( transform );
    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    std::vector<Vec2> uvs;
    std::vector<IntVec3> vertDefs;
    bool foundNormals = false;

    std::string lineText = "";
    std::ifstream fileHandle = std::ifstream( filePath );
    GUARANTEE_RECOVERABLE( fileHandle.is_open(), Stringf( "Failed to open objFile (%s)", filePath.c_str() ) );

    while( getline( fileHandle, lineText ) ) {
        Strings params = SplitStringOnDelimeter( lineText, ' ', false );

        if( (int)params.size() == 0 ) {
            continue;
        }

        if( params[0] == "v" ) {
            // it's a vertex position
            int numParams = (int)params.size();
            GUARANTEE_OR_DIE( numParams == 4, Stringf( "Invalid position in objFile (%s): %s", filePath.c_str(), lineText.c_str() ) );

            std::string vertStr = Stringf( "%s,%s,%s", params[1].c_str(), params[2].c_str(), params[3].c_str() );
            Vec3 vert = Vec3( vertStr );

            vert = transformMatrix.TransformPosition3D( vert );
            vert *= scale;

            positions.push_back( vert );
        } else if( params[0] == "vn" ) {
            // it's a normal vector
            int numParams = (int)params.size();
            GUARANTEE_OR_DIE( numParams == 4, Stringf( "Invalid normal in objFile (%s): %s", filePath.c_str(), lineText.c_str() ) );

            foundNormals = true;
            std::string normStr = Stringf( "%s,%s,%s", params[1].c_str(), params[2].c_str(), params[3].c_str() );
            Vec3 norm = Vec3( normStr );

            norm = transformMatrix.TransformVector3D( norm );
            normals.push_back( norm );
        } else if( params[0] == "vt" ) {
            // it's a texture uv
            int numParams = (int)params.size();
            GUARANTEE_OR_DIE( numParams == 3 || numParams == 4, Stringf( "Invalid UV in objFile (%s): %s", filePath.c_str(), lineText.c_str() ) );
            std::string uvStr = Stringf( "%s,%s", params[1].c_str(), params[2].c_str() );
            uvs.emplace_back( uvStr );
        } else if( params[0] == "f" ) {
            // it's a face
            int numParams = (int)params.size();
            GUARANTEE_OR_DIE( numParams == 4 || numParams == 5, Stringf( "Invalid face in objFile (%s): %s", filePath.c_str(), lineText.c_str() ) );

            int counterIndexes[6] = { 1, 2, 3, 1, 3, 4 };
            int clockwiseIndexes[6] = { 3, 2, 1, 4, 3, 1 };
            int* vertIndexes = (invert ? clockwiseIndexes : counterIndexes);

            int maxVertIndex = (numParams == 4) ? /* One tri */ 3 : /* Two tris */ 6;

            for( int vertIndex = 0; vertIndex < maxVertIndex; vertIndex++ ) {
                int paramIndex = vertIndexes[vertIndex];
                Strings indexStr = SplitStringOnDelimeter( params[paramIndex], '/' );
                int numIndexes = (int)indexStr.size();
                GUARANTEE_OR_DIE( numIndexes == 3, Stringf( "Invalid face in objFile (%s): %s", filePath.c_str(), lineText.c_str() ) );

                std::string vertStr = Stringf( "%s,%s,%s", indexStr[0].c_str(), indexStr[1].c_str(), indexStr[2].c_str() );
                IntVec3 vertDef = IntVec3( vertStr );

                int posIndex = vertDef.x;
                int uvIndex = vertDef.y;
                int normIndex = vertDef.z;

                Vec3 pos = positions[posIndex - 1]; // objFile is 1 indexed so subtract one from all values
                Vec2 uv = uvs[uvIndex - 1];
                uv.y = 1 - uv.y;
                SetUV( uv );

                if( foundNormals ) {
                    Vec3 normal = normals[normIndex - 1];
                    AddVertex( pos, normal );
                } else {
                    AddVertex( pos );
                }
            }
        }
    }

    fileHandle.close();

    if( !foundNormals ) {
        GenerateFlatNormals();
    }

    GenerateTangents();
    RemoveDuplicateVerts();
}


void CPUMesh::GenerateFlatNormals() {
    int numVerts = (int)m_verts.size();

    for( int vertIndex = 0; vertIndex < numVerts; vertIndex += 3 ) {
        VertexMaster& vert0 = m_verts[vertIndex];
        VertexMaster& vert1 = m_verts[vertIndex + 1];
        VertexMaster& vert2 = m_verts[vertIndex + 2];

        Vec3 disp0To1 = vert1.position - vert0.position;
        Vec3 disp0To2 = vert2.position - vert0.position;

        Vec3 normal = CrossProduct( disp0To1, disp0To2 );

        vert0.normal = normal;
        vert1.normal = normal;
        vert2.normal = normal;
    }
}


void CPUMesh::GenerateTangents() {
    int numIndices = (int)m_indices.size();

    if( numIndices != 0 ) {
        ERROR_RECOVERABLE( "Trying to generate tangents from an already indexed mesh" );
        return;
    }

    // Setup
    SMikkTSpaceContext mikkTContext;
    SMikkTSpaceInterface* mikkTInterface = new SMikkTSpaceInterface;
    mikkTContext.m_pUserData = this;
    mikkTContext.m_pInterface = mikkTInterface;

    // Config
    mikkTInterface->m_getNumFaces = GetNumFaces;
    mikkTInterface->m_getNumVerticesOfFace = GetNumVertsPerFaces;
    mikkTInterface->m_getPosition = GetPosition;
    mikkTInterface->m_getNormal = GetNormal;
    mikkTInterface->m_getTexCoord = GetUV;

    mikkTInterface->m_setTSpaceBasic = SetTangent;
    mikkTInterface->m_setTSpace = NULL;

    // Generate Tangents
    genTangSpaceDefault( &mikkTContext );

    // Clean up memory
    delete mikkTContext.m_pInterface;
}


void CPUMesh::RemoveDuplicateVerts() {
    int numIndices = (int)m_indices.size();

    if( numIndices != 0 ) {
        ERROR_RECOVERABLE( "Trying to remove duplicate vertices from an already indexed mesh" );
        return;
    }

    std::vector<VertexMaster> copyOfVerts = m_verts;
    m_verts.clear();
    m_indices.clear();

    int numVerts = (int)copyOfVerts.size();
    int numVertsBefore = numVerts;

    for( int vertIndex = 0; vertIndex < numVerts; vertIndex++ ) {
        int numAdded = (int)m_verts.size();
        VertexMaster& vert = copyOfVerts[vertIndex];
        bool dupFound = false;

        for( int addedIndex = 0; addedIndex < numAdded; addedIndex++ ) {
            VertexMaster& addedVert = m_verts[addedIndex];

            if( vert == addedVert ) {
                m_indices.push_back( addedIndex );
                dupFound = true;
                break;
            }
        }

        if( !dupFound ) {
            int newIndex = AddVertex( vert );
            m_indices.push_back( newIndex );
        }
    }


    int numVertsAfter = (int)m_verts.size();
    g_theDevConsole->PrintString( Stringf( "- (CPUMesh) %.0d duplicate verts removed.", numVertsBefore - numVertsAfter ), DevConsole::CHANNEL_INFO );
    g_theDevConsole->PrintString( Stringf( "- (CPUMesh) %.0d Before... %.0d After.", numVertsBefore, numVertsAfter ), DevConsole::CHANNEL_INFO );
}


int CPUMesh::GetVertexCount() const {
    return (int)m_verts.size();
}


int CPUMesh::GetIndexCount() const {
    return (int)m_indices.size();
}


int CPUMesh::GetElementCount() const {
    return (IsUsingIndexBuffer() ? GetIndexCount() : GetVertexCount());
}


bool CPUMesh::IsUsingIndexBuffer() const {
    return (GetIndexCount() > 0);
}


int CPUMesh::GetNumFaces( const SMikkTSpaceContext* pContext ) {
    CPUMesh* mesh = (CPUMesh*)pContext->m_pUserData;
    int numVerts = mesh->GetVertexCount();
    int numVertsPerFace = GetNumVertsPerFaces( pContext, 0 );

    return numVerts / numVertsPerFace;
}


int CPUMesh::GetNumVertsPerFaces( const SMikkTSpaceContext* pContext, const int iFace ) {
    UNUSED( pContext );
    UNUSED( iFace );
    return 3;
}


void CPUMesh::GetPosition( const SMikkTSpaceContext * pContext, float fvPosOut[], const int iFace, const int iVert ) {
    CPUMesh* mesh = (CPUMesh*)pContext->m_pUserData;

    const VertexMaster& vert = mesh->GetVertAtIndex( iFace, iVert );
    const Vec3& position = vert.position;

    fvPosOut[0] = position.x;
    fvPosOut[1] = position.y;
    fvPosOut[2] = position.z;
}


void CPUMesh::GetNormal( const SMikkTSpaceContext * pContext, float fvNormOut[], const int iFace, const int iVert ) {
    CPUMesh* mesh = (CPUMesh*)pContext->m_pUserData;

    const VertexMaster& vert = mesh->GetVertAtIndex( iFace, iVert );
    const Vec3& normal = vert.normal;

    fvNormOut[0] = normal.x;
    fvNormOut[1] = normal.y;
    fvNormOut[2] = normal.z;
}


void CPUMesh::GetUV( const SMikkTSpaceContext * pContext, float fvTexcOut[], const int iFace, const int iVert ) {
    CPUMesh* mesh = (CPUMesh*)pContext->m_pUserData;

    const VertexMaster& vert = mesh->GetVertAtIndex( iFace, iVert );
    const Vec2& uv = vert.uvTexCoords;

    fvTexcOut[0] = uv.x;
    fvTexcOut[1] = uv.y;
}


void CPUMesh::SetTangent( const SMikkTSpaceContext * pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert ) {
    CPUMesh* mesh = (CPUMesh*)pContext->m_pUserData;

    VertexMaster& vert = mesh->GetVertAtIndex( iFace, iVert );

    vert.tangent = Vec3( fvTangent[0], fvTangent[1], fvTangent[2] );
    vert.bitangent = fSign * CrossProduct( vert.normal, vert.tangent );
}


VertexMaster& CPUMesh::GetVertAtIndex( const int iFace, const int iVert ) {
    int vertIndex = (iFace * 3) + iVert;

    return m_verts[vertIndex];
}
