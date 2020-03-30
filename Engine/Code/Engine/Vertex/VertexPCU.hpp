#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Vertex/VertexMaster.hpp"


struct VertexPCU {
	public:
    Vec3 position = Vec3::ZERO;
    Rgba color = Rgba::WHITE;
    Vec2 uvTexCoords = Vec2::ZERO;

    VertexPCU() {};
	explicit VertexPCU( const Vec3& position, const Rgba& color, const Vec2& uvTexCoords );

    static const BufferAttribute LAYOUT[4];
    static void CopyFromMaster( void* outArray, const std::vector<VertexMaster>& inList );
};
