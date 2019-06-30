#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/IntVec2.hpp"


struct AABB2;
class TileDef;
struct Metadata;

class Tile {
    public:
    Tile( IntVec2 tileCoords, const std::string& type );
    ~Tile();

    AABB2 GetWorldBounds() const;
    const std::string& GetTileType() const;
    AABB2 GetUVs() const;
    Rgba GetTint() const;

    IntVec2 GetTileCoords() const;
    Metadata* GetMetadata() const;
    float GetDistanceField() const;
    float GetNoiseValue() const;
    
    bool AllowsSight() const;
    bool AllowsWalking() const;
    bool AllowsFlying() const;
    bool AllowsSwimming() const;

    void SetTileType( std::string type );
    void SetTileType( const TileDef* tileDef );
    void SetDistanceField( float newDistance );
    void SetNoiseValue( float newNoise );

    private:
    IntVec2 m_tileCoords = IntVec2( -1, -1 );
    const TileDef* m_tileDef = nullptr;
    Metadata* m_metadata = nullptr;
};
