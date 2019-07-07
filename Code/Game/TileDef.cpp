#include "Game/TileDef.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


int TileDef::s_numTileTypes = 0;
std::map<std::string, TileDef*> TileDef::s_tileDefs;
const SpriteSheet* TileDef::s_terrainSprites = nullptr;

TileDef::TileDef( const XMLElement& element ) {
    m_tileType = ParseXMLAttribute( element, "name", "UNKNOWN" );
    GUARANTEE_OR_DIE( m_tileType != "UNKNOWN", "(TileDef) Tile missing required attribute 'name'" );

    m_tileContext = ParseXMLAttribute( element, "context", m_tileContext );
    m_drawOrder = s_numTileTypes++;

    m_spriteCoords = ParseXMLAttribute( element, "spriteCoords", m_spriteCoords );
    s_terrainSprites->GetSpriteDef( m_spriteCoords ).GetUVs( m_uvCoords.mins, m_uvCoords.maxs );

    m_spriteTint = ParseXMLAttribute( element, "spriteTint", m_spriteTint );
    m_texelColor = ParseXMLAttribute( element, "texelColor", m_texelColor );

    m_allowsSight = ParseXMLAttribute( element, "allowsSight", m_allowsSight );
    m_allowsWalking = ParseXMLAttribute( element, "allowsWalking", m_allowsWalking );
    m_allowsFlying = ParseXMLAttribute( element, "allowsFlying", m_allowsFlying );
    m_allowsSwimming = ParseXMLAttribute( element, "allowsSwimming", m_allowsSwimming );

    if( StringICmp( m_tileContext, "edged" ) ) {
        DefineEdgeTileDefs();
    }

    g_theDevConsole->PrintString( Stringf( "(TileDef) Loaded new tileDef (%s)", m_tileType.c_str() ) );
    s_tileDefs[m_tileType] = this;
}


bool TileDef::CompareDrawOrder::operator()( const TileDef* const& tileDefA, const TileDef* const& tileDefB ) {
    return (tileDefA->m_drawOrder > tileDefB->m_drawOrder);
}


/*
bool TileDef::operator<( const TileDef& otherTileDef ) const {
    return m_drawOrder < otherTileDef.m_drawOrder;
}
*/


void TileDef::InitializeTileDefs() {
    s_terrainSprites = &(SpriteSheet::GetSpriteSheet( "Terrain" ));

    XmlDocument document = new XmlDocument();
    const XMLElement& root = ParseXMLRootElement( DATA_TILE_DEFS, document );

    const XMLElement* element = root.FirstChildElement( "TileDefinition" );

    for( element; element != nullptr; element = element->NextSiblingElement( "TileDefinition" ) ) {
        new TileDef( *element ); // Upon construction, adds self to static registry
    }
}


void TileDef::DestroyTileDefs() {

}


const TileDef* TileDef::GetTileDef( std::string tileType ) {
    std::map<std::string, TileDef*>::const_iterator tileDefIter = s_tileDefs.find( tileType );

    if( tileDefIter != s_tileDefs.end() ) {
        return tileDefIter->second;
    } else {
        return nullptr;
    }
}


const TileDef* TileDef::GetTileDefFromTexelColor( const Rgba& texelColor ) {
    std::map<std::string, TileDef*>::const_iterator tileDefIter = s_tileDefs.begin();
    for( tileDefIter; tileDefIter != s_tileDefs.end(); tileDefIter++ ) {
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


std::string TileDef::GetSpriteTexture() {
    return s_terrainSprites->GetTexturePath();
}


const SpriteSheet TileDef::GetSpriteSheet() {
    return *s_terrainSprites;
}


const std::string& TileDef::GetTileType() const {
    return m_tileType;
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


const int g_edgedWidth = 3;
const int g_edgedHeight = 6;


// ----- PRIVATE -----
TileDef::TileDef( const TileDef& defToCopy, const IntVec2& offsetCoords ) :
    m_tileType          ( Stringf( "%s_%d_%d", defToCopy.m_tileType.c_str(), offsetCoords.x, offsetCoords.y ) ),
    m_tileContext       ( ""                                        ),
    m_drawOrder         ( s_numTileTypes++                          ),
    m_spriteCoords      ( defToCopy.m_spriteCoords + offsetCoords   ),
    m_spriteTint        ( defToCopy.m_spriteTint                    ),
    m_texelColor        ( defToCopy.m_texelColor                    ), // DFS1FIXME: This could be a problem
    m_allowsSight       ( defToCopy.m_allowsSight                   ),
    m_allowsWalking     ( defToCopy.m_allowsWalking                 ),
    m_allowsFlying      ( defToCopy.m_allowsFlying                  ),
    m_allowsSwimming    ( defToCopy.m_allowsSwimming ) {

    s_terrainSprites->GetSpriteDef( m_spriteCoords ).GetUVs( m_uvCoords.mins, m_uvCoords.maxs );

    g_theDevConsole->PrintString( Stringf( "(TileDef) Loaded new tileDef (%s)", m_tileType.c_str() ) );
    s_tileDefs[m_tileType] = this;
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
