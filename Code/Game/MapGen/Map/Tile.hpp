#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Vertex/VertexUtils.hpp"


class CPUMesh;
class Map;
class TileDef;

struct AABB2;
struct Metadata;


typedef unsigned int NeighborFlag;


class Tile {
    friend class TileDef;

    public:
    Tile( IntVec2 tileCoords, const std::string& type );
    ~Tile();

    AABB2 GetWorldBounds() const;
    const std::string& GetTileType() const;
    const std::string& GetTileContext() const;

    IntVec2 GetTileCoords() const;
    Metadata* GetMetadata() const;
    Strings GetRenderTypes() const;
    bool GetHeatMap( const std::string& heatMapName, float& outHeatMapValue ) const;
    
    bool AllowsSight() const;
    bool AllowsWalking() const;
    bool AllowsFlying() const;
    bool AllowsSwimming() const;

    void SetTileType( const std::string& tileType );
    void SetTileType( const TileDef* tileDef );
    void SetHeatMap( const std::string& heatMapName, float heatMapValue );

    void AddRenderType( const std::string& tileType );
    void AddRenderType( const TileDef* tileDef );
    bool AddTypesFromNeighbors( const Map& map );

    void AddVertsToMesh( CPUMesh& builder ) const;


    private:
    IntVec2 m_tileCoords = IntVec2( -1, -1 );
    const TileDef* m_tileDef = nullptr;
    Metadata* m_metadata = nullptr;


    AABB2 GetUVs() const;
    Rgba GetTint() const;
    void GetEdgedNeighborByType( const Map& map, std::map< std::string, NeighborFlag >& edgedNeighbors ) const;
    bool AddEdgesFromNeighborFlags( std::map< std::string, NeighborFlag >& edgedNeighbors );
};
