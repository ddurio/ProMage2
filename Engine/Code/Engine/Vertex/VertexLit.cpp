#include "Engine/Vertex/VertexLit.hpp"


VertexLit::VertexLit( const Vec3& positionIn, const Rgba& colorIn, const Vec2& uvTexCoordsIn, const Vec3& tangentIn, const Vec3& bitangentIn, const Vec3& normalIn ) :
    position( positionIn ),
    color( colorIn ),
    uvTexCoords( uvTexCoordsIn ),
    tangent( tangentIn ),
    bitangent( bitangentIn ),
    normal( normalIn ) {
}


const BufferAttribute VertexLit::LAYOUT[7] = {
    BufferAttribute( "POSITION",  ATTR_TYPE_VEC3, offsetof( VertexLit, position ) ),
    BufferAttribute( "COLOR",     ATTR_TYPE_RGBA, offsetof( VertexLit, color ) ),
    BufferAttribute( "UV",        ATTR_TYPE_VEC2, offsetof( VertexLit, uvTexCoords ) ),
    BufferAttribute( "TANGENT",   ATTR_TYPE_VEC3, offsetof( VertexLit, tangent ) ),
    BufferAttribute( "BITANGENT", ATTR_TYPE_VEC3, offsetof( VertexLit, bitangent ) ),
    BufferAttribute( "NORMAL",    ATTR_TYPE_VEC3, offsetof( VertexLit, normal ) ),
    BufferAttribute() // NULL Terminated
};


void VertexLit::CopyFromMaster( void* outList, const std::vector<VertexMaster>& inList ) {
    int numInVerts = (int)inList.size();

    for( int vertIndex = 0; vertIndex < numInVerts; vertIndex++ ) {
        const VertexMaster& master = inList[vertIndex];

        ((VertexLit*)outList)[vertIndex] = VertexLit( master.position, master.color, master.uvTexCoords, master.tangent, master.bitangent, master.normal );
    }
}
