#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/IntVec2.hpp"

#include "Game/MapGen/Map/Tile.hpp"


class Actor;
class Inventory;
class Item;
class MapDef;
class Model;
class PhysicsSystem;
class RigidBody2D;
class RNG;


class Map {
    friend class MapDef;
    friend class MapGenStep;

    public:
    explicit Map( std::string mapName, std::string mapType, RNG* mapRNG, int level = 0 );
    ~Map();

    void Startup();
    void Shutdown();

    void Update( float deltaSeconds );
    void Render() const;

    int GetCurrentFloor() const;
    RNG* GetMapRNG() const;
    IntVec2 GetMapDimensions() const;
    Inventory* GetMapInventory() const;

    // Tile Interactions
    const IntVec2 GetTileCoords( const Vec2& worldCoords ) const;
    const IntVec2 GetTileCoordsForStairs( bool getStairsDown ) const;

    int GetTileIndex( int tileX, int tileY ) const;
    int GetTileIndex( const IntVec2& tileCoords ) const;
    int GetTileIndex( const Vec2& worldCoords ) const;

    const Tile& GetTile( int tileIndex ) const;
    const Tile& GetTile( int xIndex, int yIndex ) const;
    const Tile& GetTile( const IntVec2& tileCoords ) const;
    const Tile& GetTile( const Vec2& worldCoords ) const;
    bool GetTileIfValid( const Tile*& outTile, const IntVec2& tileCoords ) const;

    bool IsValidTileCoords( const IntVec2& tileCoords ) const;

    // Actors
    Actor* GetPlayer() const;
    Actor* GetActorInRange( const std::string& typeToFind, const Vec2& worldCoords, float radius ) const;
    Actor* GetActorInSight( const Actor* fromActor ) const;
    Actor* GetActorInCone( const Vec2& coneCenter, const Vec2& coneDirection, float coneMinDot, float coneRadius, Actor* excludeActor = nullptr ) const;
    bool HasLineOfSight( const Actor* fromActor, const Actor* toActor ) const;

    Actor* SpawnNewActor( std::string actorType, std::string controllerType, const Vec2& worldPosition );
    Item* SpawnLootDrop( Inventory* inventory, const Vec2& worldPosition = Vec2::ZERO, RNG* customRNG = nullptr ) const;

    bool SpawnNewActor( EventArgs& args );
    bool SpawnNewItem( EventArgs& args );

    // Lists
    void SetPlayer( Actor* player );
    void AddPlayerToMap( Actor* actor );
    void AddActorToMap( Actor* entity );

    void ClearPlayer( Actor* player );
    void RemovePlayerFromMap( Actor* actor );
    void RemoveActorFromMap( Actor* entity );

    static void ResetMaterialCreated();


    private:
    static bool s_materialCreated;

    int m_floorIndex = 0;
    std::string m_mapName = "";
    std::string m_mapType = "";
    const MapDef* m_mapDef = nullptr;

    RNG* m_mapRNG = nullptr;
    IntVec2 m_mapDimensions = IntVec2::ZERO;
    RigidBody2D* m_tilesRB = nullptr;

    std::vector<Tile> m_tiles;
    Model* m_model = nullptr;

    std::vector< Actor* > m_actors;
    Actor* m_player = nullptr;

    Map* m_self = this; // Needed for inventory to work - mainly for when players move between maps
    Inventory* m_inventory = nullptr;

    std::vector< float >        m_lootPercents;
    std::vector< std::string >  m_lootTypes;


    bool HandleEnemyDeath( EventArgs& args );
    void SpawnEnemyLootDrop( Actor* enemy ) const;

    void CreateTerrainMesh();
    void CreateLootTable();
    void CollectGarbage();
};
