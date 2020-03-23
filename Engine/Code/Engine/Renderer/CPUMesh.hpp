#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Vertex/VertexMaster.hpp"
#include "Engine/Vertex/VertexUtils.hpp"

enum CoordinatePlane {
    COORD_PLANE_XY,
    COORD_PLANE_XZ,
    COORD_PLANE_ZY
};

struct AABB2;
//struct AABB3;
struct Capsule2;
struct Capsule3;
struct IntVec3;
struct OBB2;
struct OBB3;
struct Rgba;
struct SMikkTSpaceContext;
struct Vec2;
struct Vec3;


class CPUMesh {
    friend class GPUMesh;

    public:
    void Clear();

    void SetColor( const Rgba& color );
    void SetUV( const Vec2& uv );

    int AddVertex( const VertexMaster& vertex );
    int AddVertex( const Vec3& position, const Vec3& normal = Vec3::SCREEN );
    void AddVertexArray( const VertexList& verts );

    void SetTangent( const Vec3& tangent );
    void SetBitangent( const Vec3& bitangent );
    void SetNormal( const Vec3& normal );

    void SetDefaultMaterial( const std::string& materialName );
    std::string GetDefaultMaterial() const;

    IntVec3 AddTriangle( const Vec3& position0, const Vec3& position1, const Vec3& position2 );
    IntVec3 AddTriangle( const Vec2& position0, const Vec2& position1, const Vec2& position2 );

    void AddQuad( const AABB2& quad, const AABB2& uvs = AABB2::UVZEROTOONE );
    void AddQuad( const OBB2& box, const AABB2& uvs = AABB2::UVZEROTOONE );
    void AddQuad( const Vec2& mins, const Vec2& maxs, const AABB2& uvs = AABB2::UVZEROTOONE, CoordinatePlane plane = COORD_PLANE_XY );
    void AddQuad( const Vec3& mins, const Vec3& maxs, const AABB2& uvs = AABB2::UVZEROTOONE, CoordinatePlane plane = COORD_PLANE_XY );
    void AddQuad( const Vec2& positionTL, const Vec2& positionTR, const Vec2& positionBL, const Vec2& positionBR, const AABB2& uvs = AABB2::UVZEROTOONE );
    void AddQuad( const Vec3& positionTL, const Vec3& positionTR, const Vec3& positionBL, const Vec3& positionBR, const AABB2& uvs = AABB2::UVZEROTOONE );
    void AddQuadEdge( OBB2 box, float thickness );

    void AddRoundedQuad( const OBB2& box, float radius, const AABB2& uvs = AABB2::UVZEROTOONE );
    void AddRoundedQuadEdge( const OBB2& box, float radius, float thickness );

    void AddBox( const Vec3& mins, const Vec3& maxs );
    void AddBox( const OBB3& box );

    void AddCapsule( const Capsule2& capsule );
    void AddCapsule( const Capsule3& capsule );
    void AddCapsuleEdge( const Capsule2& capsule, float thickness );
    //void AddCapsuleEdge( const Capsule3& capsule, float thickness );

    void AddLine( const Vec2& start, const Vec2& end, float thickness );
    void AddLine( const Vec3& start, const Vec3& end, float thickness );

    void AddArrow( const Vec2& start, const Vec2& end, float thickness );

    void AddCone( const Vec3& position, float height, float radius, const Vec3& upVector = Vec3::UP, int numWedges = 32 );
    void AddHourGlass( const Vec3& position, float height, float radius, const Vec3& upVector = Vec3::UP, int numWedges = 32 );
    void AddCylinder( const Vec3& position, float height, float radius, const Vec3& upVector = Vec3::UP, int numWedges = 32 );
    void AddNonUniformCylinder( const Vec3& position, float height, float radiusBot, float radiusTop, const Vec3& upVector = Vec3::UP, int numWedges = 32 );

    void AddCircle( const Vec2& center, float radius, int numWedges = 32 );
    void AddCircle( const Vec3& center, float radius, const Vec3& normal = Vec3::SCREEN, int numWedges = 32 );

    void AddRing( const Vec2& center, float radiusInner, float radiusOuter, int numWedges = 32 );
    void AddPartialRing( const Vec2& center, float radiusInner, float radiusOuter, float degreesStart, float degreesEnd, int numWedgesForCircle = 32 );

    void AddUVSphere( const Vec3& center, float radius, int numWedges = 32, int numSlices = 16 );

    void AddIndexedTriangle( int index0, int index1, int index2 );
    void AddIndexedQuad( int indexTL, int indexTR, int indexBL, int indexBR );

    void LoadObjFile( const std::string& filePath, bool invert, float scale, const std::string& transform );
    void GenerateFlatNormals();
    void GenerateTangents();
    void RemoveDuplicateVerts();

    int GetVertexCount() const;
    int GetIndexCount() const;
    int GetElementCount() const;

    bool IsUsingIndexBuffer() const;

    // MikkT Space Functions
    static int GetNumFaces( const SMikkTSpaceContext* pContext );
    static int GetNumVertsPerFaces( const SMikkTSpaceContext* pContext, const int iFace );
    static void GetPosition( const SMikkTSpaceContext * pContext, float fvPosOut[], const int iFace, const int iVert );
    static void GetNormal( const SMikkTSpaceContext * pContext, float fvNormOut[], const int iFace, const int iVert );
    static void GetUV( const SMikkTSpaceContext * pContext, float fvTexcOut[], const int iFace, const int iVert );
    static void SetTangent( const SMikkTSpaceContext * pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert );

    private:
    std::vector<VertexMaster> m_verts;
    std::vector<int> m_indices;
    VertexMaster m_stamp = VertexMaster();
    std::string m_defaultMaterial = "";

    // MikkT Space Utility Function
    VertexMaster& GetVertAtIndex( const int iFace, const int iVert );
};
