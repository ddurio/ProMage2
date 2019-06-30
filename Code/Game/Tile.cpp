#include "Game/Tile.hpp"

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/VertexUtils.hpp"

#include "Game/TileDef.hpp"
#include "Game/Metadata.hpp"


Tile::Tile( IntVec2 tileCoords, const std::string& type ) :
    m_tileCoords(tileCoords) {
    m_metadata = new Metadata();
    SetTileType( type );
}


Tile::~Tile() {
    delete m_metadata;
    m_metadata = nullptr;
}


AABB2 Tile::GetWorldBounds() const {
    Vec2 mins( (float)m_tileCoords.x, (float)m_tileCoords.y );
    Vec2 maxs = mins + Vec2( 1.f, 1.f );
    return AABB2( mins, maxs );
}


const std::string& Tile::GetTileType() const {
    return m_tileDef->GetTileType();
}


AABB2 Tile::GetUVs() const {
    AABB2 uvs;
    m_tileDef->GetUVs( uvs.mins, uvs.maxs );

    return uvs;
}


Rgba Tile::GetTint() const {
    return m_tileDef->GetSpriteTint();
}


IntVec2 Tile::GetTileCoords() const {
    return m_tileCoords;
}


Metadata* Tile::GetMetadata() const {
    return m_metadata;
}


float Tile::GetDistanceField() const {
    return m_metadata->m_distanceField;
}


float Tile::GetNoiseValue() const {
    return m_metadata->m_noiseValue;
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


void Tile::SetTileType( std::string type ) {
    m_tileDef = TileDef::GetTileDef( type );
}


void Tile::SetTileType( const TileDef* tileDef ) {
    m_tileDef = tileDef;
}


void Tile::SetDistanceField( float newDistance ) {
    m_metadata->m_distanceField = newDistance;
}


void Tile::SetNoiseValue( float newNoise ) {
    m_metadata->m_noiseValue = newNoise;
}
