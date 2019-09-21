#include "Game/MapGen/Map/TileDef.hpp"

#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/MapGen/Map/Tile.hpp"
#include "Game/MapGen/Map/Metadata.hpp"


int TileDef::s_numTileTypes = 0;
const SpriteSheet* TileDef::s_terrainSprites = nullptr;


bool TileDef::CompareDrawOrder::operator()( const TileDef* const& tileDefA, const TileDef* const& tileDefB ) {
    return (tileDefA->m_drawOrder > tileDefB->m_drawOrder);
}


void TileDef::DefineObject( Tile& theObject ) const {
    theObject.m_tileDef = this;

    theObject.m_metadata->m_renderTypes = TileQueue();

    int numTypes = (int)m_extraRenderTypes.size();

    for( int typeIndex = 0; typeIndex < numTypes; typeIndex++ ) {
        const std::string& extraStr = m_extraRenderTypes[typeIndex];
        theObject.AddRenderType( extraStr );
    }
}


const SpriteSheet TileDef::GetTerrainSprites() {
    return *s_terrainSprites;
}


const TileDef* TileDef::GetTileDefFromTexelColor( const Rgba& texelColor ) {
    std::map<std::string, TileDef*>::const_iterator tileDefIter = s_definitions.begin();

    for( tileDefIter; tileDefIter != s_definitions.end(); tileDefIter++ ) {
        TileDef* tileDef = tileDefIter->second;
        const Rgba tileTexelColor = tileDef->GetTexelColor();

        if( texelColor == tileTexelColor ) {
            return tileDef;
        }
    }

    return nullptr;
}


std::string TileDef::GetTileTypeFromTexelColor( const Rgba& texelColor ) {
    const TileDef* tileDef = GetTileDefFromTexelColor( texelColor );
    return tileDef->GetTileType();
}


const std::string& TileDef::GetTileType() const {
    return m_defType;
}


const std::string& TileDef::GetTileContext() const {
    return m_tileContext;
}


void TileDef::GetUVs( Vec2& uvMins, Vec2& uvMaxs ) const {
    uvMins = m_uvCoords.mins;
    uvMaxs = m_uvCoords.maxs;
}


const Rgba& TileDef::GetSpriteTint() const {
    return m_spriteTint;
}


const Rgba& TileDef::GetTexelColor() const {
    return m_texelColor;
}


int TileDef::GetDrawOrder() const {
    return m_drawOrder;
}


const Strings& TileDef::GetExtraRenderTypes() const {
    return m_extraRenderTypes;
}


bool TileDef::AllowsSight() const {
    return m_allowsSight;
}


bool TileDef::AllowsWalking() const {
    return m_allowsWalking;
}


bool TileDef::AllowsFlying() const {
    return m_allowsFlying;
}


bool TileDef::AllowsSwimming() const {
    return m_allowsSwimming;
}


static const int g_edgedWidth = 3;
static const int g_edgedHeight = 6;


// ----- PRIVATE -----
TileDef::TileDef( const XMLElement& element ) {
    // Static Initializations
    s_defClass = "TileDef";
    s_terrainSprites = &(SpriteSheet::GetSpriteSheet( "Terrain" ));

    // Type
    m_defType = ParseXMLAttribute( element, "name", m_defType );
    GUARANTEE_OR_DIE( m_defType != "", "(TileDef) Tile missing required attribute 'name'" );

    // Render Variables
    m_tileContext = ParseXMLAttribute( element, "context", m_tileContext );
    m_drawOrder = s_numTileTypes++;

    m_spriteCoords = ParseXMLAttribute( element, "spriteCoords", m_spriteCoords );
    s_terrainSprites->GetSpriteDef( m_spriteCoords ).GetUVs( m_uvCoords.mins, m_uvCoords.maxs );

    std::string extraRenderStr = ParseXMLAttribute( element, "extraRenderTypes", "" );
    m_extraRenderTypes = SplitStringOnDelimeter( extraRenderStr, ',', false );

    m_spriteTint = ParseXMLAttribute( element, "spriteTint", m_spriteTint );
    m_texelColor = ParseXMLAttribute( element, "texelColor", m_texelColor );

    // Map Variables
    m_allowsSight = ParseXMLAttribute( element, "allowsSight", m_allowsSight );
    m_allowsWalking = ParseXMLAttribute( element, "allowsWalking", m_allowsWalking );
    m_allowsFlying = ParseXMLAttribute( element, "allowsFlying", m_allowsFlying );
    m_allowsSwimming = ParseXMLAttribute( element, "allowsSwimming", m_allowsSwimming );

    // Context Sensitivity
    if( StringICmp( m_tileContext, "edged" ) ) {
        DefineEdgeTileDefs();
    }
}


TileDef::TileDef( const TileDef& defToCopy, const IntVec2& offsetCoords ) :
    m_tileContext       ( ""                                        ),
    m_drawOrder         ( s_numTileTypes++                          ),
    m_spriteCoords      ( defToCopy.m_spriteCoords + offsetCoords   ),
    m_spriteTint        ( defToCopy.m_spriteTint                    ),
    m_texelColor        ( defToCopy.m_texelColor                    ), // THESISFIXME: This could be a problem
    m_allowsSight       ( defToCopy.m_allowsSight                   ),
    m_allowsWalking     ( defToCopy.m_allowsWalking                 ),
    m_allowsFlying      ( defToCopy.m_allowsFlying                  ),
    m_allowsSwimming    ( defToCopy.m_allowsSwimming ) {

    m_defType = Stringf( "%s_%d_%d", defToCopy.m_defType.c_str(), offsetCoords.x, offsetCoords.y );
    s_terrainSprites->GetSpriteDef( m_spriteCoords ).GetUVs( m_uvCoords.mins, m_uvCoords.maxs );

    AddDefinition( this );
}



void TileDef::DefineEdgeTileDefs() {
    for( int offsetY = 0; offsetY < g_edgedHeight; offsetY++ ) {
        for( int offsetX = 0; offsetX < g_edgedWidth; offsetX++ ) {
            if( offsetX == 1 && offsetY == 5 ) {
                continue; // "this" will take place of 1,5
            }

            new TileDef( *this, IntVec2( offsetX, offsetY ) );
        }
    }

    m_spriteCoords += IntVec2( 1, 5 );
    s_terrainSprites->GetSpriteDef( m_spriteCoords ).GetUVs( m_uvCoords.mins, m_uvCoords.maxs );
}
