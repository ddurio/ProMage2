#include "Game/MapGen/GenSteps/MGS_DistanceField.hpp"

#include "Game/MapGen/Map/Map.hpp"


MGS_DistanceField::MGS_DistanceField( const XMLElement& element ) :
    MapGenStep(element) {
    m_movementType = ParseXMLAttribute( element, "movementType", m_movementType );
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
            Tile& tile = GetTile( theMap, tileX, tileY );

            if( IsTileValid( theMap, tile ) ) {
                tile.SetHeatMap( "Distance", 0.f );
                openTiles.push_back( IntVec2( tileX, tileY ) );
            } else {
                tile.SetHeatMap( "Distance", 999999.f );
            }
        }
    }
}


void MGS_DistanceField::OpenNeighbors( Map& map, std::vector<IntVec2>& openTiles, std::vector<bool>& openedTiles, const IntVec2& tileCoords ) const {
    Tile& tile = GetTile( map, tileCoords.x, tileCoords.y );

    float distance;
    tile.GetHeatMap( "Distance", distance );
    distance += 1.f;

    IntVec2 neighborOffsets[4] = {
        IntVec2( -1,  0 ), // Left
        IntVec2(  1,  0 ), // Right
        IntVec2(  0,  1 ), // Up
        IntVec2(  0, -1 ), // Down
    };

    // For each cardinal neighbor
    for( int offsetIndex = 0; offsetIndex < 4; offsetIndex++ ) {
        IntVec2 neighborCoords = tileCoords + neighborOffsets[offsetIndex];

        if( map.IsValidTileCoords( neighborCoords ) ) {
            int neighborIndex = map.GetTileIndex( neighborCoords );
            Tile& neighbor = GetTile( map, neighborIndex );

            float neighborDist;
            neighbor.GetHeatMap( "Distance", neighborDist );

            if( IsNeighborTileValid( neighbor ) && distance < neighborDist ) {
                neighbor.SetHeatMap( "Distance", distance );

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
        tile.GetHeatMap( "Distance", distance );

        std::string distanceString = Stringf( "(MGS_DistanceField) Tile (%s) distance = %f", tileCoords.GetAsString().c_str(), distance );

        g_theDevConsole->PrintString( distanceString, s_mgsChannel );
        //DebuggerPrintf( distanceString.c_str() );
    }
}
*/
