#include "Game/Map.hpp"

#include "Engine/Core/DebugDraw.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Ray2.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Model.hpp"
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


Map::Map( std::string mapName, std::string mapType, RNG* mapRNG, int level /*= 0 */ ) :
    m_mapName(mapName),
    m_mapType(mapType),
    m_mapRNG(mapRNG),
    m_floorIndex(level) {
}


Map::~Map() {
    EngineCommon::ClearVector( m_actors );
    CLEAR_POINTER( m_inventory );
    CLEAR_POINTER( m_model );

    g_thePhysicsSystem->DestroyRigidBody( m_tilesRB );
    g_theEventSystem->Unsubscribe( "enemyDeath", this, &Map::HandleEnemyDeath );
}


void Map::Startup() {
    m_inventory = new Inventory( nullptr, m_self, false, true ); // Must be before Define, could be needed

    m_mapDef = MapDef::GetMapDef( m_mapType );
    m_mapDef->Define( *this );
    CreateTerrainMesh();
    CreateLootTable();

    m_inventory->SpawnNewItem( "Slippers", Vec2( 4.5f, 4.5f ) );
    m_inventory->SpawnNewItem( "Shoes", Vec2( 5.5f, 5.5f ) );

    g_theEventSystem->Subscribe( "enemyDeath", this, &Map::HandleEnemyDeath );
}


void Map::Shutdown() {
    CLEAR_POINTER( m_model );
}


void Map::Update( float deltaSeconds ) {
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
    // Render the terrain
    g_theRenderer->DrawModel( m_model );

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


RNG* Map::GetMapRNG() const {
    return m_mapRNG;
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


Actor* Map::GetActorInSight( const Actor* fromActor ) const {
    int numActors = (int)m_actors.size();

    for( int actorIndex = 0; actorIndex < numActors; actorIndex++ ) {
        Actor* toActor = m_actors[actorIndex];

        if( toActor != nullptr && toActor->IsAlive() && toActor->IsKillable()
            && toActor != fromActor && HasLineOfSight( fromActor, toActor ) ) {
            // DFS1FIXME: Can add faction comparison here if desired
            return toActor;
        }
    }

    return nullptr;
}


Actor* Map::GetActorInCone( const Vec2& coneCenter, const Vec2& coneDirection, float coneMinDot, float coneRadius, Actor* excludeActor /*= nullptr */ ) const {
    Vec2 coneDirNorm = coneDirection.GetNormalized();
    float coneRadiusSqr = coneRadius * coneRadius;
    int numActors = (int)m_actors.size();

    int bestIndex = -1;
    float bestFactor = -1.f;

    for( int actorIndex = 0; actorIndex < numActors; actorIndex++ ) {
        Actor* actor = m_actors[actorIndex];

        if( actor != nullptr && actor->IsAlive() && actor->IsKillable() ) {
            if( actor == excludeActor ) {
                continue;
            }

            Vec2 actorPos = actor->GetPosition();
            Vec2 displacement = (actorPos - coneCenter).GetNormalized();

            // Is actor within cone radius
            float dispLengthSqr = displacement.GetLengthSquared();
            bool actorWithinRadius = (dispLengthSqr <= coneRadiusSqr);

            // Is actor within cone angle
            float dispDotCone = DotProduct( displacement, coneDirNorm );
            bool actorWithinAngle = (dispDotCone >= coneMinDot);

            // Valid hit actor, but is it the best so far?
            if( actorWithinRadius && actorWithinAngle ) {
                float radiusFactor = RangeMapFloat( dispLengthSqr, 0.f, coneRadiusSqr, 1.f, 0.f );
                float angleFactor = RangeMapFloat( dispDotCone, coneMinDot, 1.f, 0.f, 1.f );

                float actorFactor = radiusFactor * angleFactor;

                if( bestIndex < 0 || actorFactor > bestFactor ) {
                    bestIndex = actorIndex;
                    bestFactor = actorFactor;
                }
            }
        }
    }


    if( bestIndex < 0 ) {
        return nullptr;
    }

    return m_actors[bestIndex];
}


bool Map::HasLineOfSight( const Actor* fromActor, const Actor* toActor ) const {
    Vec2 fromPos  = fromActor->GetPosition();
    Vec2 toPos    = toActor->GetPosition();
    Ray2 sightRay = Ray2::MakeFromPoints( fromActor->GetPosition(), toActor->GetPosition() );

    Vec2 displacement = toPos - fromPos;
    float maxDistSqr = DotProduct( displacement, displacement );

    for( int yIndex = 0; yIndex < m_mapDimensions.y; yIndex++ ) {
        for( int xIndex = 0; xIndex < m_mapDimensions.x; xIndex++ ) {
            // Check if tile blocks sight
            const Tile& tile = GetTileFromTileCoords( xIndex, yIndex );

            if( tile.AllowsSight() ) {
                continue;
            }

            // Get tile AABB
            Vec2 mins = Vec2( (float)xIndex, (float)yIndex );
            Vec2 maxs = mins + Vec2::ONE;
            AABB2 tileBounds = AABB2( mins, maxs );

            // Raycast
            float inters[2] = { 0.f };
            int numHits = sightRay.Raycast( tileBounds, inters );

            if( numHits > 0 && maxDistSqr >= inters[0] * inters[0] ) {
                return false;
            }
        }
    }

    return true;
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


bool Map::HandleEnemyDeath( EventArgs& args ) {
    Actor* deadActor = nullptr;
    deadActor = args.GetValue<Actor>( "actor", deadActor );

    if( deadActor == nullptr ) {
        return true;
    }

    // Run deathTimer first
    if( deadActor->m_deathTimer == nullptr ) {
        Clock* gameClock = g_theGame->GetGameClock();
        deadActor->m_deathTimer = new Timer( gameClock );
        deadActor->m_deathTimer->Start( 3.f );
    } else {
        CLEAR_POINTER( deadActor->m_deathTimer );
        deadActor->m_isGarbage = true;

        SpawnLootDrop( deadActor->GetPosition() );
    }

    return false;
}


void Map::SpawnLootDrop( const Vec2& worldPosition ) const {
    float diceRoll = g_RNG->GetRandomFloatZeroToOne();
    int numItems = (int)m_lootTypes.size();

    for( int itemIndex = 0; itemIndex < numItems; itemIndex++ ) {
        const float& requiredRoll = m_lootPercents[itemIndex];

        if( diceRoll >= requiredRoll ) {
            std::string itemType = m_lootTypes[itemIndex];
            m_inventory->SpawnNewItem( itemType, worldPosition );
            return;
        }
    }
}


bool Map::s_materialCreated = false;


void Map::CreateTerrainMesh() {
    int numTilesX = m_mapDimensions.x;
    int numTilesY = m_mapDimensions.y;

    CPUMesh builder;

    Vec3 vertOffsets[9] = {
        Vec3(  0.f,     0.f,    0.f ),  // Bottom Left
        Vec3(  0.5f,    0.f,    0.f ),  // Bottom Center
        Vec3(  1.f,     0.f,    0.f ),  // Bottom Right
        Vec3(  0.f,     0.5f,   0.f ),  // Center Left
        Vec3(  0.5f,    0.5f,   0.f ),  // Center Center
        Vec3(  1.f,     0.5f,   0.f ),  // Center Right
        Vec3(  0.f,     1.f,    0.f ),  // Top Left
        Vec3(  0.5f,    1.f,    0.f ),  // Top Center
        Vec3(  1.f,     1.f,    0.f ),  // Top Right
    };

    Vec2 uvValues[9] = {
        ALIGN_BOTTOM_LEFT,
        ALIGN_BOTTOM_CENTER,
        ALIGN_BOTTOM_RIGHT,
        ALIGN_CENTER_LEFT,
        ALIGN_CENTER,
        ALIGN_CENTER_RIGHT,
        ALIGN_TOP_LEFT,
        ALIGN_TOP_CENTER,
        ALIGN_TOP_RIGHT
    };


    for( int tileY = 0; tileY < numTilesY; tileY++ ) {
        for( int tileX = 0; tileX < numTilesX; tileX++ ) {
            int tileIndex = tileY * numTilesX + tileX;
            const Tile& tile = m_tiles[tileIndex];

            Vec3 center = Vec3( (float)tileX, (float)tileY, 0.f );
            int indexList[9] = {};

            // Construct all the verts
            for( int offsetIndex = 0; offsetIndex < 9; offsetIndex++ ) {
                // Position
                const Vec3& offset = vertOffsets[offsetIndex];
                Vec3 vertPos = center + offset;

                // Color
                Rgba tint = tile.GetTint();

                // UV - invert min/max
                AABB2 uvs = tile.GetUVs();
                Vec2 alignment = uvValues[offsetIndex];
                const Vec2 vertUV = uvs.GetPointWithin( alignment );

                // Add Vert
                builder.SetColor( tint );
                builder.SetUV( vertUV );
                indexList[offsetIndex] = builder.AddVertex( vertPos );
            }

            int indexBL = indexList[0];
            int indexBC = indexList[1];
            int indexBR = indexList[2];
            int indexCL = indexList[3];     // TL---TC---TR
            int indexCC = indexList[4];     // CL   CC   CR
            int indexCR = indexList[5];     // BL---BC---BR
            int indexTL = indexList[6];
            int indexTC = indexList[7];
            int indexTR = indexList[8];

            builder.AddIndexedQuad( indexCL, indexCC, indexBL, indexBC );
            builder.AddIndexedQuad( indexCC, indexCR, indexBC, indexBR );
            builder.AddIndexedQuad( indexTL, indexTC, indexCL, indexCC );
            builder.AddIndexedQuad( indexTC, indexTR, indexCC, indexCR );
        }
    }

    // Mesh
    GPUMesh* mesh = new GPUMesh( g_theRenderer );
    mesh->CopyVertsFromCPUMesh( &builder, "BuiltIn/Unlit" );

    // Material
    Material* material = g_theRenderer->GetOrCreateMaterial( "Terrain" );

    if( !s_materialCreated ) {
        std::string terrainTexture = TileDef::GetSpriteTexture();
        material->SetTexture( terrainTexture );
        material->SetShader( "BuiltIn/Unlit" );

        s_materialCreated = true;
    }

    m_model = new Model();
    m_model->SetMesh( mesh );
    m_model->SetMaterial( "Terrain" );
    m_model->SetModelMatrix( Matrix44::IDENTITY );
}


void Map::CreateLootTable() {
    const std::map< std::string, Definition<Item>*, StringCmpCaseI >& itemDefs = Definition<Item>::GetAllDefinitions();
    auto itemDefIter = itemDefs.begin();

    float totalUnscaledPercents = 0.f;
    std::vector< float > itemPercents;

    for( itemDefIter; itemDefIter != itemDefs.end(); itemDefIter++ ) {
        const Definition<Item>* itemDef = itemDefIter->second;
        IntRange floorRange = itemDef->GetProperty( "dropFloors", IntRange::NEGONE );

        if( !floorRange.IsIntInRange( m_floorIndex ) ) {
            continue;
        }

        // Valid item.. scale to find percent chance
        const std::string& itemType = itemDefIter->first;
        std::string bias = itemDef->GetProperty( "dropBias", std::string() );
        float itemPercent = 0.f;

        // Map floor range into 0 -> 1 based on bias
        if( StringICmp( bias, "early" ) ) {
            itemPercent = RangeMapFloat( (float)m_floorIndex, (float)floorRange.min, (float)floorRange.max, 1.f, 0.001f );
        } else if( StringICmp( bias, "late" ) ) {
            itemPercent = RangeMapFloat( (float)m_floorIndex, (float)floorRange.min, (float)floorRange.max, 0.001f, 1.f );
        } else if( StringICmp( bias, "middle" ) ) {
            itemPercent = RangeMapFloat( (float)m_floorIndex, (float)floorRange.min, (float)floorRange.max, -1.f, 1.f );
            itemPercent = abs( itemPercent ); // Valley at middle
            itemPercent = 1.f - itemPercent;  // Peak at middle
        } else {
            ERROR_AND_DIE( Stringf( "(Map) Unrecognized drop bias '%s' during loot table constructions", bias.c_str() ) );
        }

        totalUnscaledPercents += itemPercent;
        itemPercents.push_back( itemPercent );
        m_lootTypes.push_back( itemType );
    }

    // Scale by total percent and accumulate to get percent breakdown
    int numItems = (int)m_lootTypes.size();
    float cumulativePercent = 1.f;

    for( int itemIndex = 0; itemIndex < numItems; itemIndex++ ) {
        float itemPercent = itemPercents[itemIndex] / totalUnscaledPercents;
        cumulativePercent = ClampFloat( cumulativePercent - itemPercent, 0.f, 1.f );

        m_lootPercents.push_back( cumulativePercent );
    }
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
