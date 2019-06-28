#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

#include "Game/Entity.hpp"
#include "Game/Tile.hpp"


class Actor;
class Inventory;
class MapDef;
class RigidBody2D;


class Map {
    friend class MapDef;
    friend class MapGenStep;

    public:
    explicit Map( std::string mapName, std::string mapType, RNG* mapRNG );
    ~Map();

    void Startup();
    void Shutdown();

    void Update( float deltaSeconds );
    void Render() const;

    IntVec2 GetMapDimensions() const;
    const IntVec2 GetTileCoordsFromWorldCoords( const Vec2& worldCoords ) const;
    const IntVec2 GetTileCoordsForStairs( bool getStairsDown ) const;

    int GetTileIndexFromTileCoords( const IntVec2& tileCoords ) const;
    int GetTileIndexFromTileCoords( int xIndex, int yIndex ) const;
    int GetTileIndexFromWorldCoords( const Vec2& worldCoords ) const;
    const Tile& GetTileFromTileIndex( int tileIndex ) const;
    const Tile& GetTileFromTileCoords( const IntVec2& tileCoords ) const;
    const Tile& GetTileFromTileCoords( int xIndex, int yIndex ) const;
    const Tile& GetTileFromWorldCoords( const Vec2& worldCoords ) const;
    //std::vector< const Tile& > GetSurroundingTilesFromWorldCoords( const Vec2& worldCoords ) const;
    //auto GetSurroundingTilesFromWorldCoords( const Vec2& worldCoords ) const -> const Tile(&)[8];

    Inventory* GetMapInventory() const;
    Actor* GetPlayer() const;
    Actor* GetActorInRange( const std::string& typeToFind, const Vec2& worldCoords, float radius ) const;
    Actor* GetActorInSight( const Actor* fromActor ) const;
    Actor* GetActorInCone( const Vec2& coneCenter, const Vec2& coneDirection, float coneMinDot, float coneRadius, Actor* excludeActor = nullptr ) const;
    bool HasLineOfSight( const Actor* fromActor, const Actor* toActor ) const;

    bool IsValidTileCoords( const IntVec2& tileCoords ) const;

    Actor* SpawnNewActor( std::string actorType, std::string controllerType, const Vec2& worldPosition );

    void SetPlayer( Actor* player );
    void AddPlayerToMap( Actor* actor );
    void AddActorToMap( Actor* entity );
    void AddActorToList( Actor* entity, std::vector< Actor* >& list );

    void ClearPlayer( Actor* player );
    void RemovePlayerFromMap( Actor* actor );
    void RemoveActorFromMap( Actor* entity );
    void RemoveActorFromList( Actor* entity, std::vector< Actor* >& list );

    private:
    std::string m_mapName = "";
    std::string m_mapType = "";
    RNG* m_mapRNG = nullptr;

    IntVec2 m_mapDimensions = IntVec2( 0, 0 );
    const MapDef* m_mapDef = nullptr;

    std::vector<Tile> m_tiles;
    VertexList m_mapVerts;
    RigidBody2D* m_tilesRB = nullptr;

    std::vector< Actor* > m_actors;
    Actor* m_player = nullptr;

    Map* m_self = this; // Needed for inventory to work
    Inventory* m_inventory = nullptr;

    void UpdateMapVerts( float deltaSeconds );
    void CollectGarbage();
};
