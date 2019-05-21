#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

#include "Game/Entity.hpp"
#include "Game/Tile.hpp"


class Actor;
class Inventory;
class MapDef;


class Map {
    friend class MapDef;
    friend class MapGenStep;

    public:
    explicit Map( std::string mapName, std::string mapType );
    ~Map();

    void Startup();
    void Shutdown();

    void Update( float deltaSeconds );
    void Render() const;

    IntVec2 GetMapDimensions() const;
    const IntVec2 GetTileCoordsFromWorldCoords( const Vec2& worldCoords ) const;
    int GetTileIndexFromTileCoords( const IntVec2& tileCoords ) const;
    int GetTileIndexFromTileCoords( int xIndex, int yIndex ) const;
    int GetTileIndexFromWorldCoords( const Vec2& worldCoords ) const;
    const Tile& GetTileFromTileIndex( int tileIndex ) const;
    const Tile& GetTileFromTileCoords( const IntVec2& tileCoords ) const;
    const Tile& GetTileFromTileCoords( int xIndex, int yIndex ) const;
    const Tile& GetTileFromWorldCoords( const Vec2& worldCoords ) const;
    Inventory* GetMapInventory() const;
    bool IsValidTileCoords( const IntVec2& tileCoords ) const;

    Actor* SpawnNewActor( std::string actorType, const Vec2& worldPosition, int playerID = -1 );

    void AddEntityToMap( Entity& entity );
    void AddEntityToList( Entity& entity, EntityList& list );

    void RemoveEntityFromMap( Entity& entity );
    void RemoveEntityFromList( Entity& entity, EntityList& list );

    private:
    std::string m_mapName = "";
    std::string m_mapType = "";
    IntVec2 m_mapDimensions = IntVec2( 0, 0 );
    const MapDef* m_mapDef = nullptr;

    std::vector<Tile> m_tiles;
    VertexList m_mapVerts;

    EntityList m_entities;

    Inventory* m_inventory = nullptr;

    void UpdateMapVerts( float deltaSeconds );
    void UpdateCollision();
};
