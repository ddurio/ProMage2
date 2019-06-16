#include "Game/MapDef.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"

#include "Game/Map.hpp"
#include "Game/MapGenStep.hpp"
#include "Game/Tile.hpp"


std::map<std::string, MapDef*> MapDef::s_mapDefs;

MapDef::MapDef( const XMLElement& element ) {
    m_mapType = ParseXMLAttribute( element, "name", "UNKNOWN" );

    m_tileFillType = ParseXMLAttribute( element, "fillTile", "UNKNOWN" );
    m_tileEdgeType = ParseXMLAttribute( element, "edgeTile", "UNKNOWN" );

    m_width = ParseXMLAttribute( element, "width", IntRange::ZERO );
    m_height = ParseXMLAttribute( element, "height", IntRange::ZERO );


    const XMLElement* stepsRoot = element.FirstChildElement( "GenerationSteps" );
    const XMLElement* genStep = stepsRoot->FirstChildElement();

    for( genStep; genStep != nullptr; genStep = genStep->NextSiblingElement() ) {
        MapGenStep* step = MapGenStep::CreateMapGenStep( *genStep );
        m_mapGenSteps.push_back( step );
    }

    g_theDevConsole->PrintString( Stringf( "(MapDef) Loaded new MapDef (%s)", m_mapType.c_str() ) );

    s_mapDefs[m_mapType] = this;
}


MapDef::~MapDef() {
    int numSteps = (int)m_mapGenSteps.size();

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        delete m_mapGenSteps[stepIndex];
        m_mapGenSteps[stepIndex] = nullptr;
    }
}


void MapDef::InitializeMapDefs() {
    XmlDocument document = new XmlDocument();
    const XMLElement& root = ParseXMLRootElement( DATA_MAP_DEFS, document );

    const char* tagName = "MapDefinition";
    const XMLElement* element = root.FirstChildElement( tagName );

    for( element; element != nullptr; element = element->NextSiblingElement( tagName ) ) {
        new MapDef( *element ); // Upon construction, adds self to static registry
    }
}


const MapDef* MapDef::GetMapDef( std::string mapType ) {
    std::map<std::string, MapDef*>::const_iterator mapDefIter = s_mapDefs.find( mapType );

    if( mapDefIter != s_mapDefs.end() ) {
        return mapDefIter->second;
    } else {
        return nullptr;
    }
}


void MapDef::Define( Map& map ) const {
    int mapWidth = map.m_mapRNG->GetRandomIntInRange( m_width );
    int mapHeight = map.m_mapRNG->GetRandomIntInRange( m_height );

    map.m_mapDimensions = IntVec2( mapWidth, mapHeight );
    // Ensures no tile destructor gets called during creation
    // Avoids 'delete metadata' getting called and dangling pointers
    map.m_tiles.reserve( mapWidth * mapHeight );

    // Create Tiles, Set to Fill Type
    for( int tileCoordY = 0; tileCoordY < mapHeight; tileCoordY ++ ) {
        for( int tileCoordX = 0; tileCoordX < mapWidth; tileCoordX++ ) {
            map.m_tiles.emplace_back( IntVec2( tileCoordX, tileCoordY ), m_tileFillType );
        }
    }

    // Set Border to Edge Type
    if( m_tileEdgeType != "UNKNOWN" ) {
        int tileIndex;

        // Make top and bottom edge Tiles
        for( int x = 0; x < mapWidth; x++ ) {
            // Bottom Row
            tileIndex = x;
            map.m_tiles[tileIndex].SetTileType( m_tileEdgeType );

            // Top Row
            tileIndex = (mapHeight - 1) * mapWidth + x;
            map.m_tiles[tileIndex].SetTileType( m_tileEdgeType );
        }

        // Make left and right edge Tiles
        for( int y = 0; y < mapHeight; y++ ) {
            // Left Column
            tileIndex = mapWidth * y;
            map.m_tiles[tileIndex].SetTileType( m_tileEdgeType );

            // Right Column
            tileIndex = (mapWidth * y) + (mapWidth - 1);
            map.m_tiles[tileIndex].SetTileType( m_tileEdgeType );
        }
    }

    // Run Each MapGenStep
    int numSteps = (int)m_mapGenSteps.size();
    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        m_mapGenSteps[stepIndex]->Run( map );
    }

    // Setup Tile Colliders
    for( int tileCoordY = 0; tileCoordY < mapHeight; tileCoordY ++ ) {
        for( int tileCoordX = 0; tileCoordX < mapWidth; tileCoordX++ ) {
            const Tile& tile = map.GetTileFromTileCoords( tileCoordX, tileCoordY );

            if( !tile.AllowsWalking() ) {
                if( map.m_tilesRB == nullptr ) {
                    map.m_tilesRB = g_thePhysicsSystem->CreateNewRigidBody( 1.f );
                }

                AABB2 worldBounds = tile.GetWorldBounds();
                map.m_tilesRB->AddCollider( worldBounds );
            }
        }
    }
}
