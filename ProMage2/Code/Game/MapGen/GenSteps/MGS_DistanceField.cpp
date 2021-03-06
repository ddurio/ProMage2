#include "Game/MapGen/GenSteps/MGS_DistanceField.hpp"

#include "Game/MapGen/Map/Map.hpp"


MGS_DistanceField::MGS_DistanceField( const XMLElement& element, const Strings& motifHierarchy ) :
    MapGenStep( element, motifHierarchy ) {
    m_heatMapName  = ParseXMLAttribute( element, "heatMapName",  m_motifVars,  m_motifHierarchy,  m_heatMapName  );
    m_movementType = ParseXMLAttribute( element, "movementType", m_motifVars,  m_motifHierarchy,  m_movementType );
    m_maxDistance  = ParseXMLAttribute( element, "maxDistance",  m_motifVars,  m_motifHierarchy,  m_maxDistance  );
}


MGS_DistanceField::MGS_DistanceField( const Strings& motifHierarchy ) :
    MapGenStep( motifHierarchy ) {
}


void MGS_DistanceField::SaveToXml( XmlDocument& document, XMLElement& element ) const {
    element.SetName( "DistanceField" );
    MapGenStep::SaveToXml( document, element );

    std::string varName = "movementType";

    if( m_motifVars.IsNameSet( varName ) ) {
        std::string motifVar = "%" + m_motifVars.GetValue( varName, "" ) + "%";
        element.SetAttribute( varName.c_str(), motifVar.c_str() );
    } else if( m_movementType != "Walk" ) {
        element.SetAttribute( varName.c_str(), m_movementType.c_str() );
    }

    varName = "maxDistance";
    if( m_motifVars.IsNameSet( varName ) ) {
        std::string motifVar = "%" + m_motifVars.GetValue( varName, "" ) + "%";
        element.SetAttribute( varName.c_str(), motifVar.c_str() );
    } else if( m_maxDistance != INVALID_DISTANCE ) {
        element.SetAttribute( varName.c_str(), m_maxDistance );
    }
}


bool MGS_DistanceField::RecalculateMotifVars( EventArgs& args ) {
    MapGenStep::RecalculateMotifVars( args );

    std::string attrName = args.GetValue( MAPGEN_ARG_ATTR_NAME, "" );
    std::string varName = m_motifVars.GetValue( attrName, "" );
    bool calcAllVars = StringICmp( attrName, MAPGEN_ARG_RECALC_ALL );

    if( !calcAllVars && varName == "" ) {
        return false;
    }

    if( calcAllVars || StringICmp( attrName, "movementType" ) ) {
        varName = m_motifVars.GetValue( "movementType", "" );
        m_movementType = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_movementType );
    }
    
    if( calcAllVars || StringICmp( attrName, "maxDistance" ) ) {
        varName = m_motifVars.GetValue( "maxDistance", "" );
        m_maxDistance = MotifDef::GetVariableValue( m_motifHierarchy, varName, m_maxDistance );
    }

    return false;
}


void MGS_DistanceField::RunOnce( Map& theMap ) const {
    IntVec2 mapDimensions = theMap.GetMapDimensions();
    int numTiles = mapDimensions.x * mapDimensions.y;

    std::vector<IntVec2> openTiles;
    std::vector< bool > openedTiles;

    openTiles.reserve( numTiles );
    openedTiles.resize( numTiles, false );

    SetupDistanceField( theMap, openTiles );
    std::vector<IntVec2>::iterator openTilesIter = openTiles.begin();

    while( openTilesIter != openTiles.end() ) {
        const IntVec2& tileCoords = *openTilesIter;

        OpenNeighbors( theMap, openTiles, openedTiles, tileCoords );
        openTilesIter = openTiles.erase( openTilesIter ); // Remove current tile and update iterator
    }

    //EchoDistanceField( map );
}


void MGS_DistanceField::SetupDistanceField( Map& theMap, std::vector<IntVec2>& openTiles ) const {
    IntVec2 mapDimensions = theMap.GetMapDimensions();
    
    // For each tile (by X and Y) set initial distanceField value
    for( int tileY = 0; tileY < mapDimensions.y; tileY++ ) {
        for( int tileX = 0; tileX < mapDimensions.x; tileX++ ) {
            if( !theMap.IsValidTileCoords( IntVec2( tileX, tileY ) ) ) {
                continue;
            }

            Tile& tile = GetTile( theMap, tileX, tileY );

            if( IsTileValid( theMap, tile ) ) {
                tile.SetHeatMap( m_heatMapName, 0.f );
                openTiles.push_back( IntVec2( tileX, tileY ) );
            } else {
                tile.SetHeatMap( m_heatMapName, INVALID_DISTANCE );
            }
        }
    }
}


void MGS_DistanceField::OpenNeighbors( Map& theMap, std::vector<IntVec2>& openTiles, std::vector<bool>& openedTiles, const IntVec2& tileCoords ) const {
    Tile& tile = GetTile( theMap, tileCoords.x, tileCoords.y );

    float distance = INVALID_DISTANCE;
    tile.GetHeatMap( m_heatMapName, distance );
    distance += 1.f;

    if( distance > m_maxDistance ) {
        return;
    }

    IntVec2 neighborOffsets[4] = {
        IntVec2( -1,  0 ), // Left
        IntVec2(  1,  0 ), // Right
        IntVec2(  0,  1 ), // Up
        IntVec2(  0, -1 ), // Down
    };

    // For each cardinal neighbor
    for( int offsetIndex = 0; offsetIndex < 4; offsetIndex++ ) {
        IntVec2 neighborCoords = tileCoords + neighborOffsets[offsetIndex];

        if( theMap.IsValidTileCoords( neighborCoords ) ) {
            int neighborIndex = theMap.GetTileIndex( neighborCoords );
            Tile& neighbor = GetTile( theMap, neighborIndex );

            float neighborDist;
            neighbor.GetHeatMap( m_heatMapName, neighborDist );

            if( IsNeighborTileValid( neighbor ) && distance < neighborDist ) {
                ChangeTileDistance( theMap, neighborIndex, distance );

                if( !openedTiles[neighborIndex] ) {
                    openTiles.push_back( neighborCoords );
                    openedTiles[neighborIndex] = true;
                }
            }
        }
    }
}


bool MGS_DistanceField::IsNeighborTileValid( const Tile& tile ) const {
    if( StringICmp( m_movementType, "Fly" ) ) {
        return tile.AllowsFlying();
    } else if( StringICmp( m_movementType, "Sight" ) ) {
        return tile.AllowsSight();
    } else if( StringICmp( m_movementType, "Swim" ) ) {
        return tile.AllowsSwimming();
    } else if( StringICmp( m_movementType, "Walk" ) ) {
        return tile.AllowsWalking();
    } else {
        std::string errorMsg = Stringf( "(MGS_DistanceField) Unknown movement type '%s'", m_movementType.c_str() );
        ERROR_RECOVERABLE( errorMsg.c_str() );
    }

    return false;
}


void MGS_DistanceField::ChangeTileDistance( Map& theMap, int tileIndex, float distance ) const {
    Tile& tile = GetTile( theMap, tileIndex );

    tile.SetHeatMap( m_heatMapName, distance ); // more easily done directly, but important for editor to get custom results called too
    ChangeTile( theMap, tileIndex );
}


Strings MGS_DistanceField::GetMovementTypes() {
    Strings movementTypes = {
        "Fly",
        "Sight",
        "Swim",
        "Walk"
    };

    return movementTypes;
}


/*
void MGS_DistanceField::EchoDistanceField( Map& map ) const {
    IntVec2 mapDimensions = map.GetMapDimensions();
    int numTiles = mapDimensions.x * mapDimensions.y;

    for( int tileIndex = 0; tileIndex < numTiles; tileIndex++ ) {
        const Tile& tile = map.GetTile( tileIndex );
        IntVec2 tileCoords = tile.GetTileCoords();
        float distance;
        tile.GetHeatMap( m_heatMapName, distance );

        std::string distanceString = Stringf( "(MGS_DistanceField) Tile (%s) distance = %f", tileCoords.GetAsString().c_str(), distance );

        g_theDevConsole->PrintString( distanceString, s_mgsChannel );
        //DebuggerPrintf( distanceString.c_str() );
    }
}
*/
