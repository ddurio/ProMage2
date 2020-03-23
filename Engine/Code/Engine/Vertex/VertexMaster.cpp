#include "Engine/Vertex/VertexMaster.hpp"


VertexMaster::VertexMaster( const Vec3& positionIn, const Rgba& colorIn, const Vec2& uvTexCoordsIn, const Vec3& tangentIn, const Vec3& bitangentIn, const Vec3& normalIn ) :
    position( positionIn ),
    color( colorIn ),
    uvTexCoords( uvTexCoordsIn ),
    tangent( tangentIn ),
    bitangent( bitangentIn ),
    normal( normalIn ) {
}


bool VertexMaster::operator==( const VertexMaster& vertToCompare ) const {
    bool posMatch = (position == vertToCompare.position);
    bool colorMatch = (color == vertToCompare.color);
    bool uvMatch = (uvTexCoords == vertToCompare.uvTexCoords);
    bool tanMatch = (tangent == vertToCompare.tangent);
    bool bitanMatch = (bitangent == vertToCompare.bitangent);
    bool normMatch = (normal == vertToCompare.normal);

    return (posMatch && colorMatch && uvMatch && tanMatch && bitanMatch && normMatch);
}
