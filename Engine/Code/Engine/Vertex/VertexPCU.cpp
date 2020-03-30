#include "Engine/Vertex/VertexPCU.hpp"


VertexPCU::VertexPCU( const Vec3& positionIn, const Rgba& colorIn, const Vec2& uvTexCoordsIn ) :
	position(positionIn),
	color(colorIn),
	uvTexCoords(uvTexCoordsIn) {
}


const BufferAttribute VertexPCU::LAYOUT[4] = {
    BufferAttribute( "POSITION", ATTR_TYPE_VEC3, offsetof( VertexPCU, position ) ),
    BufferAttribute( "COLOR",    ATTR_TYPE_RGBA, offsetof( VertexPCU, color ) ),
    BufferAttribute( "UV",       ATTR_TYPE_VEC2, offsetof( VertexPCU, uvTexCoords ) ),
    BufferAttribute() // NULL Terminated
};


void VertexPCU::CopyFromMaster( void* outList, const std::vector<VertexMaster>& inList ) {
    int numInVerts = (int)inList.size();

    for( int vertIndex = 0; vertIndex < numInVerts; vertIndex++ ) {
        const VertexMaster& master = inList[vertIndex];

        ((VertexPCU*)outList)[vertIndex] = VertexPCU( master.position, master.color, master.uvTexCoords );
    }
}
