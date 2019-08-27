#include "Game/MapGenStep_DistanceField.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/IntVec2.hpp"

#include "Game/Map.hpp"
#include "Game/Tile.hpp"


MapGenStep_DistanceField::MapGenStep_DistanceField( const XMLElement& element ) :
    MapGenStep(element) {
    m_movementType = ParseXMLAttribute( element, "movementType", m_movementType );
}


void MapGenStep_DistanceField::RunOnce( Map& map ) const {
    std::vector<IntVec2> openTiles;
    IntVec2 mapDimensions = map.GetMapDimensions();
    int numTiles = mapDimensions.x * mapDimensions.y;
    openTiles.reserve( numTiles );

    ResetDistanceField( map, openTiles );

    std::vector<IntVec2>::iterator openTilesIter = openTiles.begin();

    for( openTilesIter; openTilesIter != openTiles.end(); openTilesIter ) {
        IntVec2 tileCoords = *openTilesIter;

        OpenNeighbors( map, openTiles, tileCoords );

        // Remove current tile and update iterator
        openTilesIter = openTiles.erase( openTilesIter );
    }

    //EchoDistanceField( map );
}


void MapGenStep_DistanceField::ResetDistanceField( Map& map, std::vector<IntVec2>& openTiles ) const {
    IntVec2 mapDimensions = map.GetMapDimensions();
    
    // For each tile (by X and Y) set initial distanceField value
    for( int tileY = 0; tileY < mapDimensions.y; tileY++ ) {
        for( int tileX = 0; tileX < mapDimensions.x; tileX++ ) {
            int tileIndex = map.GetTileIndexFromTileCoords( tileX, tileY );
            Tile& tile = GetTile( map, tileIndex );

            if( MapGenStep::IsTileValid(tile) ) {
                tile.SetDistanceField( 0.f );
                openTiles.push_back( IntVec2( tileX, tileY ) );
            } else {
                tile.SetDistanceField( 999999.f );
            }
        }
    }
}


void MapGenStep_DistanceField::OpenNeighbors( Map& map, std::vector<IntVec2>& openTiles, const IntVec2& tileCoords ) const {
    int tileIndex = map.GetTileIndexFromTileCoords( tileCoords );
    Tile& tile = GetTile( map, tileIndex );
    float distance = tile.GetDistanceField() + 1.0f;

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
            int neighborIndex = map.GetTileIndexFromTileCoords( neighborCoords );
            Tile& neighbor = GetTile( map, neighborIndex );

            if( IsTileValid( neighbor ) && distance < neighbor.GetDistanceField() ) {
                neighbor.SetDistanceField( distance );

                if( !VectorContains( openTiles, neighborCoords ) ) {
                    openTiles.push_back( neighborCoords );
                }
            }
        }
    }
}


// Used to check validity of pathing through a tile
// Base class version instead used to determine goals / starting points of algorithm
bool MapGenStep_DistanceField::IsTileValid( const Tile& tile ) const {
    if( m_movementType == "Fly" ) {
        return tile.AllowsFlying();
    } else if( m_movementType == "Sight" ) {
        return tile.AllowsSight();
    } else if( m_movementType == "Swim" ) {
        return tile.AllowsSwimming();
    } else if( m_movementType == "Walk" ) {
        return tile.AllowsWalking();
    }

    // ThesisFIXME: Error on unknown movement type...
    return false;
}


bool MapGenStep_DistanceField::VectorContains( const std::vector<IntVec2>& coordVec, const IntVec2& coords ) const {
    std::vector<IntVec2>::const_iterator vecIter = coordVec.begin();

    for( vecIter; vecIter != coordVec.end(); vecIter++ ) {
        const IntVec2& compare = *vecIter;

        if( coords == compare ) {
            return true;
        }
    }

    return false;
}


void MapGenStep_DistanceField::EchoDistanceField( Map& map ) const {
    IntVec2 mapDimensions = map.GetMapDimensions();
    int numTiles = mapDimensions.x * mapDimensions.y;

    for( int tileIndex = 0; tileIndex < numTiles; tileIndex++ ) {
        const Tile& tile = map.GetTileFromTileIndex( tileIndex );
        IntVec2 tileCoords = tile.GetTileCoords();
        float distance = tile.GetDistanceField();

        std::string distanceString = Stringf( "(MGS_DistanceField) Tile (%s) distance = %f", tileCoords.GetAsString().c_str(), distance );

        g_theDevConsole->PrintString( distanceString );
        //DebuggerPrintf( distanceString.c_str() );
    }
}
