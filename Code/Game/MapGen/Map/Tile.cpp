#include "Game/MapGen/Map/Tile.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/CPUMesh.hpp"

#include "Game/MapGen/Map/TileDef.hpp"
#include "Game/MapGen/Map/Map.hpp"
#include "Game/MapGen/Map/Metadata.hpp"


const NeighborFlag   NEIGHBOR_EMPTY       = 0u;
const NeighborFlag   NEIGHBOR_TOP_LEFT    = BIT_FLAG( 1 );
const NeighborFlag   NEIGHBOR_TOP_CENTER  = BIT_FLAG( 2 );
const NeighborFlag   NEIGHBOR_TOP_RIGHT   = BIT_FLAG( 3 );
const NeighborFlag   NEIGHBOR_MID_LEFT    = BIT_FLAG( 4 );
const NeighborFlag   NEIGHBOR_MID_RIGHT   = BIT_FLAG( 5 );
const NeighborFlag   NEIGHBOR_BOT_LEFT    = BIT_FLAG( 6 );
const NeighborFlag   NEIGHBOR_BOT_CENTER  = BIT_FLAG( 7 );
const NeighborFlag   NEIGHBOR_BOT_RIGHT   = BIT_FLAG( 8 );


NeighborFlag g_offsetToFlag[3][3] = {
    NEIGHBOR_BOT_LEFT,  NEIGHBOR_BOT_CENTER,    NEIGHBOR_BOT_RIGHT,
    NEIGHBOR_MID_LEFT,  NEIGHBOR_EMPTY,         NEIGHBOR_MID_RIGHT,
    NEIGHBOR_TOP_LEFT,  NEIGHBOR_TOP_CENTER,    NEIGHBOR_TOP_RIGHT
};


NeighborFlag g_neighborCases[12] = {            // Description of sprite coordinate
    NEIGHBOR_TOP_CENTER | NEIGHBOR_MID_LEFT,    // Concave TL
    NEIGHBOR_TOP_CENTER | NEIGHBOR_MID_RIGHT,   // Concave TR
    NEIGHBOR_MID_LEFT   | NEIGHBOR_BOT_CENTER,  // Concave BL
    NEIGHBOR_MID_RIGHT  | NEIGHBOR_BOT_CENTER,  // Concave BR
    NEIGHBOR_BOT_CENTER,                        // Convex Flat Top
    NEIGHBOR_MID_RIGHT,                         // Convex Flat Left
    NEIGHBOR_MID_LEFT,                          // Convex Flat Right
    NEIGHBOR_TOP_CENTER,                        // Convex Flat Bot
    NEIGHBOR_BOT_RIGHT,                         // Convex Corner TL
    NEIGHBOR_BOT_LEFT,                          // Convex Corner TR
    NEIGHBOR_TOP_RIGHT,                         // Convex Corner BL
    NEIGHBOR_TOP_LEFT,                          // Convex Corner BR
};


NeighborFlag g_neighborFlagClears[12] = {       // Flags to clear based on g_neighborCases
    NEIGHBOR_TOP_LEFT  | NEIGHBOR_TOP_CENTER | NEIGHBOR_TOP_RIGHT | NEIGHBOR_MID_LEFT   | NEIGHBOR_BOT_LEFT,    // Top row and left column
    NEIGHBOR_TOP_LEFT  | NEIGHBOR_TOP_CENTER | NEIGHBOR_TOP_RIGHT | NEIGHBOR_MID_RIGHT  | NEIGHBOR_BOT_RIGHT,   // Top row and right column
    NEIGHBOR_TOP_LEFT  | NEIGHBOR_MID_LEFT   | NEIGHBOR_BOT_LEFT  | NEIGHBOR_BOT_CENTER | NEIGHBOR_BOT_RIGHT,   // Bot row and left column
    NEIGHBOR_TOP_RIGHT | NEIGHBOR_MID_RIGHT  | NEIGHBOR_BOT_LEFT  | NEIGHBOR_BOT_CENTER | NEIGHBOR_BOT_RIGHT,   // Bot row and right column
    NEIGHBOR_BOT_LEFT  | NEIGHBOR_BOT_CENTER | NEIGHBOR_BOT_RIGHT,                                              // Bot row
    NEIGHBOR_TOP_RIGHT | NEIGHBOR_MID_RIGHT  | NEIGHBOR_BOT_RIGHT,                                              // Right column
    NEIGHBOR_TOP_LEFT  | NEIGHBOR_MID_LEFT   | NEIGHBOR_BOT_LEFT,                                               // Right column
    NEIGHBOR_TOP_LEFT  | NEIGHBOR_TOP_CENTER | NEIGHBOR_TOP_RIGHT,                                              // Bot row
    NEIGHBOR_BOT_RIGHT,
    NEIGHBOR_BOT_LEFT,
    NEIGHBOR_TOP_RIGHT,
    NEIGHBOR_TOP_LEFT
};


std::string g_neighborRenderSuffix[12] = {
    "1_0", // Concave TL
    "2_0", // Concave TR
    "1_1", // Concave BL
    "2_1", // Concave BR,
    "1_2", // Convex flat top
    "0_3", // Convex flat left
    "2_3", // Convex flat right
    "1_4", // Convex flat bot
    "0_2", // Convex curve TL
    "2_2", // Convex curve TR
    "0_4", // Convex curve BL
    "2_4", // Convex curve BR
};


Tile::Tile( IntVec2 tileCoords, const std::string& type ) :
    m_tileCoords(tileCoords) {
    m_metadata = new Metadata();
    SetTileType( type );
}


Tile::~Tile() {
    CLEAR_POINTER( m_metadata );
}


AABB2 Tile::GetWorldBounds() const {
    Vec2 mins( (float)m_tileCoords.x, (float)m_tileCoords.y );
    Vec2 maxs = mins + Vec2( 1.f, 1.f );
    return AABB2( mins, maxs );
}


const std::string& Tile::GetTileType() const {
    return m_tileDef->GetTileType();
}


const std::string& Tile::GetTileContext() const {
    return m_tileDef->GetTileContext();
}


IntVec2 Tile::GetTileCoords() const {
    return m_tileCoords;
}


Metadata* Tile::GetMetadata() const {
    return m_metadata;
}


bool Tile::GetHeatMap( const std::string& heatMapName, float& out_HeatMapValue ) const {
    bool heatMapExists = m_metadata->IsHeatMapSet( heatMapName );

    if( heatMapExists ) {
        out_HeatMapValue = m_metadata->m_heatMaps[heatMapName];
    }

    return heatMapExists;
}


bool Tile::AllowsSight() const {
    return m_tileDef->AllowsSight();
}


bool Tile::AllowsWalking() const {
    return m_tileDef->AllowsWalking();
}


bool Tile::AllowsFlying() const {
    return m_tileDef->AllowsFlying();
}


bool Tile::AllowsSwimming() const {
    return m_tileDef->AllowsSwimming();
}


void Tile::SetTileType( const std::string& tileType ) {
    const TileDef* tileDef = TileDef::GetDefinition( tileType );
    SetTileType( tileDef );
}


void Tile::SetTileType( const TileDef* tileDef ) {
    if( m_tileDef != tileDef ) {
        tileDef->DefineObject( *this );
    }
}


void Tile::SetHeatMap( const std::string& heatMapName, float heatMapValue ) {
    m_metadata->m_heatMaps[heatMapName] = heatMapValue;
}


void Tile::AddRenderType( const std::string& tileType ) {
    const TileDef* tileDef = TileDef::GetDefinition( tileType );
    AddRenderType( tileDef );
}


void Tile::AddRenderType( const TileDef* tileDef ) {
    m_metadata->m_renderTypes.push( tileDef );
}


void Tile::AddTypesFromNeighbors( const Map& map ) {
    std::map< std::string, NeighborFlag > edgedNeighbors;
    GetEdgedNeighborByType( map, edgedNeighbors );
    AddEdgesFromNeighborFlags( edgedNeighbors );
}


void Tile::AddVertsToMesh( CPUMesh& builder ) const {
    // Base Tile Type
    AABB2 tileBounds = GetWorldBounds();
    Rgba tint;
    AABB2 uvs;

    // Additional render tiles (edged tiles / stairs)
    TileQueue renderTypes = m_metadata->m_renderTypes; // Intentionally makes a copy
    renderTypes.push( m_tileDef );

    while( !renderTypes.empty() ) {
        const TileDef* tileDef = renderTypes.top();
        tint = tileDef->GetSpriteTint();
        tileDef->GetUVs( uvs.mins, uvs.maxs );

        builder.SetColor( tint );
        builder.AddQuad( tileBounds, uvs );

        renderTypes.pop();
    }
}


AABB2 Tile::GetUVs() const {
    AABB2 uvs;
    m_tileDef->GetUVs( uvs.mins, uvs.maxs );

    return uvs;
}


Rgba Tile::GetTint() const {
    return m_tileDef->GetSpriteTint();
}


void Tile::GetEdgedNeighborByType( const Map& map, std::map< std::string, NeighborFlag >& edgedNeighbors ) const {
    // Loop over 3x3 tiles coords centered on myself
    for( int offsetY = -1; offsetY < 2; offsetY++ ) {
        for( int offsetX = -1; offsetX < 2; offsetX++ ) {
            // Ignore myself
            if( offsetX == 0 && offsetY == 0 ) {
                continue;
            }

            IntVec2 neighborCoords = m_tileCoords + IntVec2( offsetX, offsetY );
            const Tile* neighbor = nullptr;

            // Get valid tile
            if( map.GetTileIfValid( neighbor, neighborCoords ) ) {
                const std::string& myType = GetTileType();
                const std::string& neighborType = neighbor->GetTileType();
                const std::string& neighborContext = neighbor->GetTileContext();

                // Make sure it's edged
                if( StringICmp(neighborContext, "edged" ) && myType != neighborType ) {
                    std::map< std::string, NeighborFlag >::iterator edgedIter = edgedNeighbors.find( neighborType );

                    // Add or modify neighbor flags
                    if( edgedIter == edgedNeighbors.end() ) {
                        edgedNeighbors[neighborType] = g_offsetToFlag[offsetY + 1][offsetX + 1];
                    } else {
                        edgedNeighbors[neighborType] |= g_offsetToFlag[offsetY + 1][offsetX + 1];
                    }
                }
            }
        }
    }
}


void Tile::AddEdgesFromNeighborFlags( std::map< std::string, NeighborFlag >& edgedNeighbors ) {
    std::map< std::string, NeighborFlag >::const_iterator neighborIter = edgedNeighbors.begin();

    for( neighborIter; neighborIter != edgedNeighbors.end(); neighborIter++ ) {
        const std::string& neighborType = neighborIter->first;
        NeighborFlag neighborFlag = neighborIter->second;

        while( neighborFlag != 0u ) {
            for( int caseIndex = 0; caseIndex < 12; caseIndex++ ) {
                if( (neighborFlag & g_neighborCases[caseIndex]) == g_neighborCases[caseIndex] ) {
                    // Add new render type
                    std::string renderType = Stringf( "%s_%s", neighborType.c_str(), g_neighborRenderSuffix[caseIndex].c_str() );
                    AddRenderType( renderType );

                    // Clear flags from neighborFlag
                    neighborFlag &= ~g_neighborFlagClears[caseIndex];

                    break;
                }
            }
        }
    }
}
