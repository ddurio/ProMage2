#include "Game/Map.hpp"

#include "Engine/Core/DebugDraw.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/Actor.hpp"
#include "Game/Entity.hpp"
#include "Game/Inventory.hpp"
#include "Game/MapDef.hpp"
#include "Game/Tile.hpp"
#include "Game/TileDef.hpp"


Map::Map( std::string mapName, std::string mapType ) :
    m_mapName(mapName),
    m_mapType(mapType) {
}


Map::~Map() {
    EngineCommon::ClearVector( m_entities );
    CLEAR_POINTER( m_inventory );
}


void Map::Startup() {
    m_inventory = new Inventory( this, false, true ); // Must be before Define, could be needed

    m_mapDef = MapDef::GetMapDef( m_mapType );
    m_mapDef->Define( *this );

    //SpawnNewActor( "Boy",  Vec2( 5.5f, 5.5f ) );
    SpawnNewActor( "Girl", Vec2( 2.5f, 3.5f ), 0 );

    m_inventory->SpawnNewItem( "Slippers", Vec2( 4.5f, 4.5f ) );
    m_inventory->SpawnNewItem( "Shoes", Vec2( 5.5f, 5.5f ) );
}


void Map::Shutdown() {

}


void Map::Update( float deltaSeconds ) {
    UpdateMapVerts( deltaSeconds );

    // Update entities
    int numEntities = (int)m_entities.size();

    for( int entityIndex = 0; entityIndex < numEntities; entityIndex++ ) {
        m_entities[entityIndex]->Update( deltaSeconds );
    }

    m_inventory->Update( deltaSeconds );

    //UpdateCollision();

    //CollectGarbage();
}


void Map::Render() const {
    std::string terrainTexture = TileDef::GetSpriteTexture();

    // Render the map itself
    g_theRenderer->BindTexture( terrainTexture );
    g_theRenderer->DrawVertexArray( m_mapVerts );

    // Render items on the map
    m_inventory->Render();

    // Render actors/entities
    int numEntities = (int)m_entities.size();

    for( int entityIndex = 0; entityIndex < numEntities; entityIndex++ ) {
        m_entities[entityIndex]->Render();
    }
}


IntVec2 Map::GetMapDimensions() const {
    return m_mapDimensions;
}


const IntVec2 Map::GetTileCoordsFromWorldCoords( const Vec2& worldCoords ) const {
    int tempX = ClampInt( (int)worldCoords.x, 0, m_mapDimensions.x - 1 );
    int tempY = ClampInt( (int)worldCoords.y, 0, m_mapDimensions.y - 1 );
    return IntVec2( tempX, tempY );
}


int Map::GetTileIndexFromTileCoords( const IntVec2& tileCoords ) const {
    int numTiles = (int)m_tiles.size();
    int tileIndex = (tileCoords.y * m_mapDimensions.x) + tileCoords.x;
    return ClampInt( tileIndex, 0, numTiles - 1 );
}


int Map::GetTileIndexFromTileCoords( int xIndex, int yIndex ) const {
    return GetTileIndexFromTileCoords( IntVec2( xIndex, yIndex ) );
}


int Map::GetTileIndexFromWorldCoords( const Vec2& worldCoords ) const {
    IntVec2 tileCoords = GetTileCoordsFromWorldCoords( worldCoords );
    return GetTileIndexFromTileCoords( tileCoords );
}


const Tile& Map::GetTileFromTileIndex( int tileIndex ) const {
    return m_tiles[tileIndex];
}


const Tile& Map::GetTileFromTileCoords( const IntVec2& tileCoords ) const {
    int tileIndex = GetTileIndexFromTileCoords( tileCoords );
    return m_tiles[tileIndex];
}


const Tile& Map::GetTileFromTileCoords( int xIndex, int yIndex ) const {
    int tileIndex = GetTileIndexFromTileCoords( xIndex, yIndex );
    return m_tiles[tileIndex];
}


const Tile& Map::GetTileFromWorldCoords( const Vec2& worldCoords ) const {
    IntVec2 tileCoords = GetTileCoordsFromWorldCoords( worldCoords );
    return GetTileFromTileCoords( tileCoords );
}


Inventory* Map::GetMapInventory() const {
    return m_inventory;
}


Entity* Map::GetPlayer( int playerID /*= 0 */ ) const {
    if( playerID >= 0 && playerID < 4 ) {
        return m_players[playerID];
    }

    return nullptr;
}


bool Map::IsValidTileCoords( const IntVec2& tileCoords ) const {
    return (tileCoords.x >= 0 && tileCoords.x < m_mapDimensions.x &&
            tileCoords.y >= 0 && tileCoords.y < m_mapDimensions.y);
}


Actor* Map::SpawnNewActor( std::string actorType, const Vec2& worldPosition /*= Vec2::ZERO*/, int playerID /*= -1*/ ) {
    Actor* newActor = new Actor( this, actorType, playerID );
    newActor->SetWorldPosition( worldPosition );

    AddEntityToMap( *newActor );
    newActor->Startup();

    if( playerID >= 0 && playerID < 4 ) {
        m_players[playerID] = newActor;
    }

    return newActor;
}


void Map::AddEntityToMap( Entity& entity ) {
    entity.m_map = this;

    AddEntityToList( entity, m_entities );
}


void Map::AddEntityToList( Entity& entity, EntityList& list ) {
    int numEntities = (int)list.size();

    for( int entityIndex = 0; entityIndex < numEntities; entityIndex++ ) {
        if( list[entityIndex] == nullptr ) {
            list[entityIndex] = &entity;
            return;
        }
    }

    list.push_back( &entity );
}


void Map::RemoveEntityFromMap( Entity& entity ) {
    RemoveEntityFromList( entity, m_entities );

    /*
    if( type == ENTITY_TYPE_PLAYER_PLACEHOLDER ) {
        PlayerPlaceholder* player = (PlayerPlaceholder*)&entity;
        int playerID = player->GetPlayerID();
        m_entitiesByType[ENTITY_TYPE_PLAYER_PLACEHOLDER][playerID] = nullptr;
    }
    */
}


void Map::RemoveEntityFromList( Entity& entity, EntityList& list ) {
    int numEntities = (int)list.size();

    for( int entityIndex = 0; entityIndex < numEntities; entityIndex++ ) {
        if( list[entityIndex] == &entity ) {
            list[entityIndex] = nullptr;
            return;
        }
    }
}


void Map::UpdateMapVerts( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    m_mapVerts.clear();

    // Tile Verts
    for( int i = 0; i < (int)m_tiles.size(); i++ ) {
        Tile& tile = m_tiles[i];
        //FIXME: Change to AddVerts passing by ref
        VertexList tileVerts = tile.GetVerts();
        m_mapVerts.insert( m_mapVerts.end(), tileVerts.begin(), tileVerts.end() );
    }

    /*
    // Vertical Grid Lines
    for( int i = 1; i < m_mapDimensions.x; i++ ) {
        AddVertsForLine2D( m_mapVerts, Vec2( (float)i, 0.f ), Vec2( (float)i, (float)m_mapDimensions.y ), .05f, Rgba::BLACK );
    }

    // Horizontal Grid Lines
    for( int i = 1; i < m_mapDimensions.y; i++ ) {
        AddVertsForLine2D( m_mapVerts, Vec2( 0.f, (float)i ), Vec2( (float)m_mapDimensions.x, (float)i ), .05f, Rgba::BLACK );
    }
    */
}


/*
void Map::UpdateCollision() {
    //---------------------------------
    // Update Entity v Tile Collision
    //---------------------------------
    static const int numTileOffsets = 8;
    static const IntVec2 tileOffsets[numTileOffsets] = {
        IntVec2( 0,  1 ), // North
        IntVec2( 0, -1 ), // South
        IntVec2( -1,  0 ), // West
        IntVec2( 1,  0 ), // East
        IntVec2( -1,  1 ), // Northwest
        IntVec2( 1,  1 ), // Northeast
        IntVec2( -1, -1 ), // Southwest
        IntVec2( 1, -1 )  // Southeast
    };

    Entity* entity1 = nullptr;
    Tile* tile = nullptr;
    int numEntities = (int)m_entities.size();

    for( int entityIter = 0; entityIter < numEntities; entityIter++ ) {
        entity1 = m_entities[entityIter];

        if( entity1 != nullptr && entity1->IsAlive() && !entity1->IsGarbage() ) {
            IntVec2 currentTileCoords = GetTileCoordsFromWorldCoords( entity1->GetPosition() );

            for( int tileIter = 0; tileIter < numTileOffsets; tileIter++ ) {
                int tileIndex = GetTileIndexFromTileCoords( currentTileCoords + tileOffsets[tileIter] );
                tile = &m_tiles[tileIndex];
                //entity1->OnCollisionTile( tile );
                // DFS1FIXME: Replace old collision handling with RigidBodies
            }
        }
    }
}
*/
