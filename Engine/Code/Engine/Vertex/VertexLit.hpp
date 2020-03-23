#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Vertex/VertexMaster.hpp"


struct VertexLit {
	public:
    Vec3 position = Vec3::ZERO;
    Rgba color = Rgba::WHITE;
    Vec2 uvTexCoords = Vec2::ZERO;
    Vec3 tangent   = Vec3::RIGHT;
    Vec3 bitangent = -Vec3::SCREEN;
    Vec3 normal    = Vec3::UP;

    VertexLit() {};
	explicit VertexLit( const Vec3& positionIn, const Rgba& colorIn, const Vec2& uvTexCoordsIn, const Vec3& tangentIn, const Vec3& bitangentIn, const Vec3& normalIn );

    static const BufferAttribute LAYOUT[7];
    static void CopyFromMaster( void* outList, const std::vector<VertexMaster>& inList );
};
