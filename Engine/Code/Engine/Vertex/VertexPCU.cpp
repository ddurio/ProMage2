#include "Engine/Vertex/VertexPCU.hpp"


VertexPCU::VertexPCU( const Vec3& positionIn, const Rgba& colorIn, const Vec2& uvTexCoordsIn ) :
	position(positionIn),
	color(colorIn),
	uvTexCoords(uvTexCoordsIn) {
}


VertexPCU::VertexPCU( const std::string& textIn ) {
    SetFromText( textIn );
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


void VertexPCU::SetFromText( const std::string& textIn ) {
    Strings params = SplitStringOnDelimeter( textIn, ' ' );
    int numParams = (int)params.size();
    GUARANTEE_OR_DIE( numParams == 3, Stringf( "Invalid VertexPCU setFromText value (%s)", textIn.c_str() ) );

    position = Vec3( params[0] );
    color = Rgba( params[1] );
    uvTexCoords = Vec2( params[2] );
}


std::string VertexPCU::GetAsString() const {
    std::string posStr = position.GetAsString();
    std::string colorStr = color.GetAsString();
    std::string uvStr = uvTexCoords.GetAsString();

    std::string vertStr = JoinStrings( { posStr, colorStr, uvStr }, " " );
    return vertStr;
}
