#include "Game/TileDef.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


std::map<std::string, TileDef*> TileDef::s_tileDefs;
SpriteSheet* TileDef::s_terrainSprites = nullptr;

TileDef::TileDef( const XMLElement& element ) {
    m_tileType = ParseXMLAttribute( element, "name", "UNKNOWN" );

    IntVec2 tileCoords = ParseXMLAttribute( element, "spriteCoords", IntVec2::ZERO );
    s_terrainSprites->GetSpriteDef( tileCoords ).GetUVs( m_uvCoords.mins, m_uvCoords.maxs );

    m_spriteTint     = ParseXMLAttribute( element, "spriteTint",     m_spriteTint );
    m_texelColor     = ParseXMLAttribute( element, "texelColor",     m_texelColor );

    m_allowsSight    = ParseXMLAttribute( element, "allowsSight",    m_allowsSight );
    m_allowsWalking  = ParseXMLAttribute( element, "allowsWalking",  m_allowsWalking );
    m_allowsFlying   = ParseXMLAttribute( element, "allowsFlying",   m_allowsFlying );
    m_allowsSwimming = ParseXMLAttribute( element, "allowsSwimming", m_allowsSwimming );

    g_theDevConsole->PrintString( Stringf( "(TileDef) Loaded new tileDef (%s)", m_tileType.c_str() ) );

    s_tileDefs[m_tileType] = this;
}


void TileDef::InitializeTileDefs() {
    s_terrainSprites = new SpriteSheet( TEXTURE_TERRAIN, IntVec2( 32, 32 ) );

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
    return TEXTURE_TERRAIN;
}


const SpriteSheet TileDef::GetSpriteSheet() {
    return *s_terrainSprites;
}


const std::string& TileDef::GetTileType() const {
    return m_tileType;
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
