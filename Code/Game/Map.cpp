#include "Game/Map.hpp"

#include "Engine/Core/DebugDraw.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/Inventory.hpp"
#include "Game/MapDef.hpp"
#include "Game/Metadata.hpp"
#include "Game/Tile.hpp"
#include "Game/TileDef.hpp"
#include "Game/TopDownFollowCamera.hpp"


Map::Map( std::string mapName, std::string mapType, RNG* mapRNG ) :
    m_mapName(mapName),
    m_mapType(mapType),
    m_mapRNG(mapRNG) {
}


Map::~Map() {
    EngineCommon::ClearVector( m_actors );
    CLEAR_POINTER( m_inventory );

    g_thePhysicsSystem->DestroyRigidBody( m_tilesRB );
}


void Map::Startup() {
    m_inventory = new Inventory( m_self, false, true ); // Must be before Define, could be needed

    m_mapDef = MapDef::GetMapDef( m_mapType );
    m_mapDef->Define( *this );

    m_inventory->SpawnNewItem( "Slippers", Vec2( 4.5f, 4.5f ) );
    m_inventory->SpawnNewItem( "Shoes", Vec2( 5.5f, 5.5f ) );
}


void Map::Shutdown() {

}


void Map::Update( float deltaSeconds ) {
    UpdateMapVerts( deltaSeconds );

    // Update entities
    int numActor = (int)m_actors.size();

    for( int actorIndex = 0; actorIndex < numActor; actorIndex++ ) {
        Actor* actor = m_actors[actorIndex];

        if( actor != nullptr && !actor->IsGarbage() ) {
            m_actors[actorIndex]->Update( deltaSeconds );
        }
    }

    m_inventory->Update( deltaSeconds );

    CollectGarbage();
}


void Map::Render() const {
    std::string terrainTexture = TileDef::GetSpriteTexture();

    // Render the map itself
    g_theRenderer->BindTexture( terrainTexture );
    g_theRenderer->DrawVertexArray( m_mapVerts );

    // Render items on the map
    m_inventory->Render();

    // Render actors/entities
    int numActor = (int)m_actors.size();

    for( int actorIndex = 0; actorIndex < numActor; actorIndex++ ) {
        const Actor* actor = m_actors[actorIndex];

        if( actor != nullptr ) {
            m_actors[actorIndex]->Render();
        }
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


const IntVec2 Map::GetTileCoordsForStairs( bool getStairsDown ) const {
    std::string stairTag = Stringf( "stairs%s", getStairsDown ? "Down" : "Up" );
    int numTiles = (int)m_tiles.size();

    for( int tileIndex = 0; tileIndex < numTiles; tileIndex++ ) {
        const Tile& tile = m_tiles[tileIndex];
        const Metadata* metadata = tile.GetMetadata();

        if( metadata->m_tagData.HasTags( stairTag ) ) {
            return tile.GetTileCoords();
        }
    }

    return IntVec2::NEGONE;
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


// Trailing return type (array of 8 Tile references)
/*
auto Map::GetSurroundingTilesFromWorldCoords( const Vec2& worldCoords ) const -> const Tile(&)[8] {
    IntVec2 tileCoords = GetTileCoordsFromWorldCoords( worldCoords );
}
*/


/*
std::vector< const Tile& > Map::GetSurroundingTilesFromWorldCoords( const Vec2& worldCoords ) const {
    IntVec2 centerCoords = GetTileCoordsFromWorldCoords( worldCoords );

    static const int numTileOffsets = 8;
    static const IntVec2 tileOffsets[numTileOffsets] = {
        IntVec2(  0,  1 ),  // North
        IntVec2(  0, -1 ),  // South
        IntVec2( -1,  0 ),  // West
        IntVec2(  1,  0 ),  // East
        IntVec2( -1,  1 ),  // Northwest
        IntVec2(  1,  1 ),  // Northeast
        IntVec2( -1, -1 ),  // Southwest
        IntVec2(  1, -1 )   // Southeast
    };

    std::vector< const Tile& > surroundingTiles;

    for( int offsetIndex = 0; offsetIndex < numTileOffsets; offsetIndex++ ) {
        IntVec2 offsetCoords = centerCoords + tileOffsets[offsetIndex];

        if( IsValidTileCoords( offsetCoords) ) {
            const Tile& tile = GetTileFromTileCoords( offsetCoords );
            surroundingTiles.push_back( tile );
        }
    }

    return surroundingTiles;
}
*/

Inventory* Map::GetMapInventory() const {
    return m_inventory;
}


Actor* Map::GetPlayer() const {
    return m_player;
}


Actor* Map::GetActorInRange( const std::string& typeToFind, const Vec2& worldCoords, float radius ) const {
    int numActor = (int)m_actors.size();

    float bestDistSqr = radius * radius;
    int bestIndex = -1;

    for( int actorIndex = 0; actorIndex < numActor; actorIndex++ ) {
        Actor* actor = m_actors[actorIndex];

        if( actor != nullptr ) {
            std::string actorType = actor->GetActorType();

            if( actorType == typeToFind ) {
                Vec2 actorPos = actor->GetPosition();
                Vec2 displacement = actorPos - worldCoords;
                float distSqr = displacement.GetLengthSquared();

                if( distSqr <= bestDistSqr ) {
                    bestIndex = actorIndex;
                    bestDistSqr = distSqr;
                }
            }
        }
    }

    if( bestIndex < 0 ) {
        return nullptr;
    }

    return m_actors[bestIndex];
}


bool Map::IsValidTileCoords( const IntVec2& tileCoords ) const {
    return (tileCoords.x >= 0 && tileCoords.x < m_mapDimensions.x &&
            tileCoords.y >= 0 && tileCoords.y < m_mapDimensions.y);
}


Actor* Map::SpawnNewActor( std::string actorType, std::string controllerType, const Vec2& worldPosition ) {
    Actor* newActor = new Actor( this, actorType, controllerType );
    newActor->SetWorldPosition( worldPosition );

    AddActorToMap( newActor );
    newActor->Startup();

    return newActor;
}


void Map::SetPlayer( Actor* player ) {
    m_player = player;

    TopDownFollowCamera* followCam = (TopDownFollowCamera*)g_theGame->GetGameCamera();
    followCam->SetFollowTarget( player );
}


void Map::AddPlayerToMap( Actor* actor ) {
    if( m_player != nullptr ) {
        m_player->m_isGarbage = true;
    }

    SetPlayer( actor );
    AddActorToMap( actor );
}


void Map::AddActorToMap( Actor* actor ) {
    actor->m_map = this;

    AddActorToList( actor, m_actors );
}


void Map::AddActorToList( Actor* actor, std::vector< Actor* >& list ) {
    int numActor = (int)list.size();

    for( int actorIndex = 0; actorIndex < numActor; actorIndex++ ) {
        if( list[actorIndex] == nullptr ) {
            list[actorIndex] = actor;
            return;
        }
    }

    list.push_back( actor );
}


void Map::ClearPlayer( Actor* player ) {
    if( m_player == player ) {
        m_player = nullptr;
    }
}


void Map::RemovePlayerFromMap( Actor* actor ) {
    if( m_player == actor ) {
        m_player = nullptr;
    } else {
        g_theDevConsole->PrintString( "(Map) WARNING: Actor argument does not match current player" );
    }

    RemoveActorFromMap( actor );
}


void Map::RemoveActorFromMap( Actor* actor ) {
    actor->m_map = nullptr;

    RemoveActorFromList( actor, m_actors );
}


void Map::RemoveActorFromList( Actor* actor, std::vector< Actor* >& list ) {
    int numActor = (int)list.size();

    for( int actorIndex = 0; actorIndex < numActor; actorIndex++ ) {
        if( list[actorIndex] == actor ) {
            list[actorIndex] = nullptr;
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

void Map::CollectGarbage() {
    int numActors = (int)m_actors.size();

    for( int actorIndex = 0; actorIndex < numActors; actorIndex++ ) {
        Actor*& actor = m_actors[actorIndex];
        
        if( actor != nullptr && actor->IsGarbage() ) {
            CLEAR_POINTER( actor );
        }
    }
}
