#include "Game/MapGen/Map/MapDef.hpp"

#include "Engine/Debug/Profiler.hpp"
#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"
#include "Game/MapGen/Map/Map.hpp"
#include "Game/MapGen/Map/Tile.hpp"


void MapDef::DefineObject( Map& theMap ) const {
    int mapWidth = theMap.m_mapRNG->GetRandomIntInRange( m_width );
    int mapHeight = theMap.m_mapRNG->GetRandomIntInRange( m_height );
    theMap.m_mapDimensions = IntVec2( mapWidth, mapHeight );

    DefineFillAndEdge( theMap );
    DefineFromMGS( theMap );
    DefineFromContextTiles( theMap );
    DefineTileColliders( theMap );
}


MapDef::MapDef( const XMLElement& element ) {
    // Name
    s_defClass = "MapDef";
    m_defType = ParseXMLAttribute( element, "name", "" );
    GUARANTEE_OR_DIE( m_defType != "", "(MapDef) Map missing required attribute 'name'" );

    // Tile Types
    m_tileFillType = ParseXMLAttribute( element, "fillTile", "" );
    GUARANTEE_OR_DIE( m_tileFillType != "", "(MapDef) Map missing required attribute 'fillType'" );
    m_tileEdgeType = ParseXMLAttribute( element, "edgeTile", "UNKNOWN" );

    // Size
    m_width = ParseXMLAttribute( element, "width", IntRange::ZERO );
    m_height = ParseXMLAttribute( element, "height", IntRange::ZERO );

    // GenSteps
    const XMLElement* stepsRoot = element.FirstChildElement( "GenerationSteps" );
    const XMLElement* genStep = stepsRoot->FirstChildElement();

    for( genStep; genStep != nullptr; genStep = genStep->NextSiblingElement() ) {
        MapGenStep* step = MapGenStep::CreateMapGenStep( *genStep );
        m_mapGenSteps.push_back( step );
    }
}


MapDef::~MapDef() {
    int numSteps = (int)m_mapGenSteps.size();

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        CLEAR_POINTER( m_mapGenSteps[stepIndex] );
    }
}


RNG* MapDef::GetMapRNG( const Map& theMap ) const {
    return theMap.m_mapRNG;
}


void MapDef::SetMapDimensions( Map& theMap, const IntVec2& dimensions ) const {
    theMap.m_mapDimensions = dimensions;
}


void MapDef::SetMapDef( Map& theMap ) const {
    theMap.m_mapType = m_defType;
    theMap.m_mapDef = this;
}


void MapDef::DefineFillAndEdge( Map& map ) const {
    IntVec2 mapDimensions = map.GetMapDimensions();
    int mapWidth = mapDimensions.x;
    int mapHeight = mapDimensions.y;

    // Ensures no tile destructor gets called during creation
    // Avoids 'delete metadata' getting called and dangling pointers
    map.m_tiles.reserve( mapWidth * mapHeight );

    // Create Tiles
    for( int tileX = 0; tileX < mapHeight; tileX ++ ) {
        for( int tileY = 0; tileY < mapWidth; tileY++ ) {
            std::string tileType = m_tileFillType;

            if( m_tileEdgeType != "UNKNOWN" && IsEdgeTile( tileY, tileX, mapWidth, mapHeight ) ) {
                tileType = m_tileEdgeType;
            }

            map.m_tiles.emplace_back( IntVec2( tileY, tileX ), tileType );
        }
    }
}


void MapDef::DefineFromMGS( Map& theMap ) const {
    PROFILE_SCOPE( "(MapDef) DefineFromMGS" );

    // Run Each MapGenStep
    int numSteps = (int)m_mapGenSteps.size();

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        m_mapGenSteps[stepIndex]->Run( theMap );
    }
}


void MapDef::DefineFromContextTiles( Map& theMap ) const {
    IntVec2 mapDimensions = theMap.GetMapDimensions();
    int numTiles = mapDimensions.x * mapDimensions.y;

    for( int tileIndex = 0; tileIndex < numTiles; tileIndex++ ) {
        Tile& tile = theMap.m_tiles[tileIndex];
        tile.AddTypesFromNeighbors( theMap );
    }
}


void MapDef::DefineTileColliders( Map& theMap ) const {
    IntVec2 mapDimensions = theMap.GetMapDimensions();
    int numTiles = mapDimensions.x * mapDimensions.y;

    // Setup Tile Colliders
    for( int tileIndex = 0; tileIndex < numTiles; tileIndex++ ) {
        const Tile& tile = theMap.GetTile( tileIndex );

        if( !tile.AllowsWalking() ) {
            AABB2 worldBounds = tile.GetWorldBounds();
            theMap.m_tilesRB->AddCollider( worldBounds );
        }
    }
}


bool MapDef::IsEdgeTile( int tileX, int tileY, int mapWidth, int mapHeight ) const {
    bool bottom = (tileY == 0);
    bool top = (tileY == mapHeight - 1);
    bool left = (tileX == 0);
    bool right = (tileX == mapWidth - 1);

    return (bottom || top || left || right);
}
