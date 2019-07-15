#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/IntVec2.hpp"


class CPUMesh;
class Map;
class TileDef;

struct AABB2;
struct Metadata;


typedef unsigned int NeighborFlag;


class Tile {
    public:
    Tile( IntVec2 tileCoords, const std::string& type );
    ~Tile();

    void AddTypesFromNeighbors( const Map& map );
    void AddVertsToMesh( CPUMesh& builder ) const;

    AABB2 GetWorldBounds() const;
    const std::string& GetTileType() const;
    const std::string& GetTileContext() const;

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

    void AddRenderType( const std::string& tileType );
    void AddRenderType( const TileDef* tileDef );

    private:
    IntVec2 m_tileCoords = IntVec2( -1, -1 );
    const TileDef* m_tileDef = nullptr;
    Metadata* m_metadata = nullptr;


    AABB2 GetUVs() const;
    Rgba GetTint() const;
    void GetEdgedNeighborByType( const Map& map, std::map< std::string, NeighborFlag >& edgedNeighbors ) const;
    void AddEdgesFromNeighborFlags( std::map< std::string, NeighborFlag >& edgedNeighbors );
};
