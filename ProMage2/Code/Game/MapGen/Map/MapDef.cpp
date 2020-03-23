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


std::string MapDef::GetMotif() const {
    return m_motif;
}


void MapDef::SaveToXml( XmlDocument& document, XMLElement& element ) const {
    element.SetAttribute( "name", m_defType.c_str() );
    element.SetAttribute( "fillTile", m_tileFillType.c_str() );

    if( m_tileEdgeType != "" ) {
        element.SetAttribute( "edgeTile", m_tileEdgeType.c_str() );
    }

    if( m_width != IntRange::ONE ) {
        element.SetAttribute( "width", m_width.GetAsString().c_str() );
    }

    if( m_height != IntRange::ONE ) {
        element.SetAttribute( "height", m_height.GetAsString().c_str() );
    }

    int numSteps = (int)m_mapGenSteps.size();

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        const MapGenStep* step = m_mapGenSteps[stepIndex];

        XMLElement* stepEle = document.NewElement( step->GetName().c_str() );
        element.InsertEndChild( stepEle );

        step->SaveToXml( document, *stepEle );
    }
}


void MapDef::UpdateMotifHierarchy() {
    int numSteps = (int)m_mapGenSteps.size();

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        MapGenStep* genStep = m_mapGenSteps[stepIndex];
        genStep->UpdateParentMotifs( { m_motif } );

        // Recalculate values
        EventArgs args;
        args.SetValue( MAPGEN_ARG_ATTR_NAME, MAPGEN_ARG_RECALC_ALL );

        genStep->RecalculateMotifVars( args );
    }
}


bool MapDef::RecalculateMotifVars( EventArgs& args ) {
    std::string attrName = args.GetValue( MAPGEN_ARG_ATTR_NAME, "" );
    std::string varName = m_motifVars.GetValue( attrName, "" );
    bool calcAllVars = StringICmp( attrName, MAPGEN_ARG_RECALC_ALL );

    if( !calcAllVars && varName == "" ) {
        return false;
    }

    if( calcAllVars || StringICmp( attrName, "fillTile" ) ) {
        m_tileFillType = MotifDef::GetVariableValue( { m_motif }, varName, m_tileFillType );
    } else if( calcAllVars || StringICmp( attrName, "edgeTile" ) ) {
        m_tileEdgeType = MotifDef::GetVariableValue( { m_motif }, varName, m_tileEdgeType );
    } else if( calcAllVars || StringICmp( attrName, "width" ) ) {
        m_width = MotifDef::GetVariableValue( { m_motif }, varName, m_width );
    } else if( calcAllVars || StringICmp( attrName, "height" ) ) {
        m_height = MotifDef::GetVariableValue( { m_motif }, varName, m_height );
    }

    return false;
}


MapDef* MapDef::CreateNewMapDef( const std::string& mapType, const std::string& fillType ) {
    MapDef* newDef = new MapDef( mapType, fillType );
    AddDefinition( newDef );

    return newDef;
}


// PROTECTED --------------------------------------
MapDef::MapDef( const XMLElement& element ) {
    // Name
    s_defClass     = "MapDef";
    m_defType      = ParseXMLAttribute( element, "name", m_defType );
    GUARANTEE_OR_DIE( m_defType != "", "(MapDef) MapDef missing required attribute 'name'" );

    m_motif        = ParseXMLAttribute( element, "motif", m_motif );

    // Tile Types
    m_tileFillType = ParseXMLAttribute( element, "fillTile", m_motifVars, { m_motif },   m_tileFillType );
    GUARANTEE_OR_DIE( m_tileFillType != "", "(MapDef) Map missing required attribute 'fillType'" );
    m_tileEdgeType = ParseXMLAttribute( element, "edgeTile", m_motifVars, { m_motif },   m_tileEdgeType );

    // Size
    m_width        = ParseXMLAttribute( element, "width",    m_motifVars, { m_motif },   m_width );
    m_height       = ParseXMLAttribute( element, "height",   m_motifVars, { m_motif },   m_height );

    // GenSteps
    const XMLElement* stepEle = element.FirstChildElement();

    for( stepEle; stepEle != nullptr; stepEle = stepEle->NextSiblingElement() ) {
        MapGenStep* genStep = MapGenStep::CreateMapGenStep( *stepEle, { m_motif } );
        m_mapGenSteps.push_back( genStep );
    }
}


MapDef::MapDef( const std::string& mapType, const std::string& fillType ) {
    m_defType = mapType;
    m_tileFillType = fillType;
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


Tile& MapDef::GetTile( Map& theMap, int tileIndex ) const {
    return theMap.m_tiles[tileIndex];
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

            if( m_tileEdgeType != "" && IsEdgeTile( tileY, tileX, mapWidth, mapHeight ) ) {
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
    DefineFromEdgedTiles( theMap );
    DefineFromWallTiles( theMap );
}


void MapDef::DefineFromEdgedTiles( Map& theMap ) const {
    IntVec2 mapDimensions = theMap.GetMapDimensions();
    int numTiles = mapDimensions.x * mapDimensions.y;

    for( int tileIndex = 0; tileIndex < numTiles; tileIndex++ ) {
        Tile& tile = theMap.m_tiles[tileIndex];
        tile.AddTypesFromNeighbors( theMap );
    }
}


void MapDef::DefineFromWallTiles( Map& theMap ) const {
    IntVec2 mapDimensions = theMap.GetMapDimensions();
    int numTiles = mapDimensions.x * mapDimensions.y;

    for( int tileIndex = 0; tileIndex < numTiles; tileIndex++ ) {
        Tile& tile = theMap.m_tiles[tileIndex];
        std::string tileContext = tile.GetTileContext();

        if( StringICmp( tileContext, "southWall" ) ) {
            tile.ChooseWallFromNeighbor( theMap );
        }
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
