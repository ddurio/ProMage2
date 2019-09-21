#include "Game/MapGen/GenSteps/MGS_RoomsAndPaths.hpp"

#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RNG.hpp"

#include "Game/MapGen/Map/Map.hpp"
#include "Game/MapGen/Map/Tile.hpp"


MGS_RoomsAndPaths::MGS_RoomsAndPaths( const XMLElement& element ) :
    MapGenStep(element) {

    // Rooms
    const XMLElement* roomElement    = element.FirstChildElement( "Rooms" );
    m_numRooms           = ParseXMLAttribute( *roomElement, "count",          m_numRooms );
    m_roomWidth          = ParseXMLAttribute( *roomElement, "width",          m_roomWidth );
    m_roomHeight         = ParseXMLAttribute( *roomElement, "height",         m_roomHeight );
    m_roomFloor          = ParseXMLAttribute( *roomElement, "floor",          m_roomFloor );
    m_roomWall           = ParseXMLAttribute( *roomElement, "wall",           m_roomWall );
    m_numOverlaps        = ParseXMLAttribute( *roomElement, "numOverlaps",    m_numOverlaps );

    // Paths
    const XMLElement* pathElement    = element.FirstChildElement( "Paths" );
    m_pathFloor          = ParseXMLAttribute( *pathElement, "floor",          m_pathFloor );
    m_pathLoop           = ParseXMLAttribute( *pathElement, "loop",           m_pathLoop );
    m_numExtraPaths      = ParseXMLAttribute( *pathElement, "extraCount",     m_numExtraPaths );
    m_pathStraightChance = ParseXMLAttribute( *pathElement, "straightChance", m_pathStraightChance );
}


void MGS_RoomsAndPaths::RunOnce( Map& theMap ) const {
    std::vector<IntVec2> roomPositions;
    std::vector<IntVec2> roomSizes;

    GenerateRooms( theMap, roomPositions, roomSizes );
    ChangeRoomTiles( theMap, roomPositions, roomSizes );

    std::vector<IntVec2> roomCenters;
    GetRoomCenters( roomPositions, roomSizes, roomCenters );
    GeneratePaths( theMap, roomCenters );
}


void MGS_RoomsAndPaths::GenerateRooms( Map& theMap, std::vector<IntVec2>& roomPositions, std::vector<IntVec2>& roomSizes ) const {
    RNG* mapRNG = theMap.GetMapRNG();
    IntVec2 theMapDimensions = theMap.GetMapDimensions();
    AABB2 theMapBounds = AABB2( Vec2::ZERO, Vec2( (float)theMapDimensions.x, (float)theMapDimensions.y ) );

    int numRooms    = mapRNG->GetRandomIntInRange( m_numRooms );
    int numOverlaps = mapRNG->GetRandomIntInRange( m_numOverlaps );

    for( int roomIndex = 0; roomIndex < numRooms; roomIndex++ ) {
        bool roomIsValid = false;
        int  attemptsRemaining = 1000;

        do {
            // Generate room size (plus two to account for walls)
            int width = mapRNG->GetRandomIntInRange( m_roomWidth ) + 2;
            int height = mapRNG->GetRandomIntInRange( m_roomHeight ) + 2;
            Vec2 roomDimensions( (float)width, (float)height );

            // Get random alignment
            float alignX = mapRNG->GetRandomFloatZeroToOne();
            float alignY = mapRNG->GetRandomFloatZeroToOne();
            Vec2 alignment = Vec2( alignX, alignY );

            // Convert to tile coords
            AABB2 roomBounds = theMapBounds.GetBoxWithin( roomDimensions, alignment );
            int theMapOffsetX = (int)roomBounds.mins.x;
            int theMapOffsetY = (int)roomBounds.mins.y;

            roomIsValid = AddRoomIfValid( theMap, roomPositions, roomSizes, numOverlaps, IntVec2( theMapOffsetX, theMapOffsetY ), IntVec2( width, height ) );

            if( !roomIsValid && --attemptsRemaining < 0 ) {
                std::string warningMsg = "(MGS_RoomsAndPaths) WARNING: Failed to place rooms as requested";
                g_theDevConsole->PrintString( warningMsg, s_mgsChannel | DevConsole::CHANNEL_WARNING );
                return;
            }
        } while( !roomIsValid );
    }
}


bool MGS_RoomsAndPaths::AddRoomIfValid( Map& theMap, std::vector<IntVec2>& positions, std::vector<IntVec2>& sizes, int& remainingOverlaps, const IntVec2& newPosition, const IntVec2& newSize ) const {
    int numExistingRooms = (int)positions.size();
    int overlaps = remainingOverlaps;

    // Check for rooms overlapping
    for( int roomIndex = 0; roomIndex < numExistingRooms; roomIndex++ ) {
        const IntVec2& existingPosition = positions[roomIndex];
        const IntVec2& existingSize     = sizes[roomIndex];

        if( DoRoomsOverlap( existingPosition, existingSize, newPosition, newSize ) ) {
            if( overlaps <= 0 ) {
                return false;
            }

            overlaps--;
        }
    }


    // Check for valid tiles under new room
    if( !IsAlignmentValid( theMap, newPosition, newSize ) ) {
        return false;
    }


    positions.push_back( newPosition );
    sizes.push_back( newSize );
    remainingOverlaps = overlaps;
    return true;
}


bool MGS_RoomsAndPaths::IsAlignmentValid( Map& theMap, const IntVec2& roomPosition, const IntVec2& roomSize ) const {
    int maxX = roomPosition.x + roomSize.x;
    int maxY = roomPosition.y + roomSize.y;

    for( int tileY = roomPosition.y; tileY <= maxY; tileY++ ) {
        for( int tileX = roomPosition.x; tileX <= maxX; tileX++ ) {
            const Tile* tile = nullptr;

            if( theMap.GetTileIfValid( tile, IntVec2( tileX, tileY ) ) ) {
                if( !IsTileValid( theMap, *tile ) ) {
                    return false;
                }
            }
        }
    }

    return true;
}


bool MGS_RoomsAndPaths::DoRoomsOverlap( const IntVec2& roomPositionA, const IntVec2& roomSizeA, const IntVec2& roomPositionB, const IntVec2& roomSizeB ) const {
    // A Mins and Maxs
    int roomAMinX = roomPositionA.x;
    int roomAMinY = roomPositionA.y;
    int roomAMaxX = roomAMinX + roomSizeA.x - 1;
    int roomAMaxY = roomAMinY + roomSizeA.y - 1;

    // B Mins and Maxs
    int roomBMinX = roomPositionB.x;
    int roomBMinY = roomPositionB.y;
    int roomBMaxX = roomBMinX + roomSizeB.x - 1;
    int roomBMaxY = roomBMinY + roomSizeB.y - 1;

    // Construct Ranges
    IntRange roomAX( roomAMinX, roomAMaxX );
    IntRange roomAY( roomAMinY, roomAMaxY );

    IntRange roomBX( roomBMinX, roomBMaxX );
    IntRange roomBY( roomBMinY, roomBMaxY );

    bool overlapX = roomAX.IsIntInRange( roomBMinX ) || roomAX.IsIntInRange( roomBMaxX ) || roomBX.IsIntInRange( roomAMinX ) || roomBX.IsIntInRange( roomAMaxX );
    bool overlapY = roomAY.IsIntInRange( roomBMinY ) || roomAY.IsIntInRange( roomBMaxY ) || roomBY.IsIntInRange( roomAMinY ) || roomBY.IsIntInRange( roomAMaxY );

    return (overlapX && overlapY);
}


void MGS_RoomsAndPaths::ChangeRoomTiles( Map& theMap, const std::vector<IntVec2>& roomPositions, std::vector<IntVec2>& roomSizes ) const {
    int numRooms = (int)roomPositions.size();

    for( int roomIndex = 0; roomIndex < numRooms; roomIndex++ ) {
        int minX = roomPositions[roomIndex].x;
        int minY = roomPositions[roomIndex].y;

        int maxX = minX + roomSizes[roomIndex].x - 1;
        int maxY = minY + roomSizes[roomIndex].y - 1;

        for( int tileY = minY; tileY <= maxY; tileY++ ) {
            for( int tileX = minX; tileX <= maxX; tileX++ ) {
                ChangeTile( theMap, tileX, tileY );
                Tile& tile = GetTile( theMap, tileX, tileY );

                std::string tileType = "";
                if( tileX == minX || tileX == maxX ||
                    tileY == minY || tileY == maxY ) {
                    tileType = m_roomWall;
                } else {
                    tileType = m_roomFloor;
                }

                tile.SetTileType( tileType );
            }
        }
    }
}


void MGS_RoomsAndPaths::GetRoomCenters( const std::vector<IntVec2>& positions, const std::vector<IntVec2>& sizes, std::vector<IntVec2>& centers ) const {
    int numRooms = (int)positions.size();

    for( int roomIndex = 0; roomIndex < numRooms; roomIndex++ ) {
        const IntVec2& roomMins = positions[roomIndex];
        const IntVec2& roomSize = sizes[roomIndex];
        IntVec2 center = roomMins + (roomSize / 2);

        centers.push_back( center );
    }
}


void MGS_RoomsAndPaths::GeneratePaths( Map& theMap, std::vector<IntVec2>& roomCenters ) const {
    RNG* mapRNG = theMap.GetMapRNG();
    float straightness = mapRNG->GetRandomFloatInRange( m_pathStraightChance );

    if( m_pathLoop ) {
        roomCenters.push_back( roomCenters[0] );
    }

    int numRooms = (int)roomCenters.size();
    IntVec2 pathStart = roomCenters[0];

    for( int roomIndex = 1; roomIndex < numRooms; roomIndex++ ) {
        const IntVec2& pathEnd = roomCenters[roomIndex];

        IntVec2 remainingDistance = pathEnd - pathStart;
        IntVec2 subPathStart = pathStart;

        // Apply Straightness Factor
        float subPathX = (float)remainingDistance.x * straightness;
        float subPathY = (float)remainingDistance.y * straightness;

        if( subPathX == 0 && remainingDistance.x != 0 ) {
            subPathX = ClampFloat( (float)remainingDistance.x, -1.f, 1.f );
        }

        if( subPathY == 0 && remainingDistance.y != 0 ) {
            subPathY = ClampFloat( (float)remainingDistance.y, -1.f, 1.f );
        }

        // Change Paths Tiles
        while( remainingDistance != IntVec2::ZERO ) {
            int absRemainX = abs( remainingDistance.x );
            int absRemainY = abs( remainingDistance.y );

            int pathSizeX = ClampInt( (int)subPathX, -absRemainX, absRemainX );
            int pathSizeY = ClampInt( (int)subPathY, -absRemainY, absRemainY );
            IntVec2 pathSize( pathSizeX, pathSizeY );

            ChangePathTiles( theMap, subPathStart, pathSize );
            subPathStart += pathSize;
            remainingDistance -= pathSize;
        }

        pathStart = pathEnd;
    }
}


void MGS_RoomsAndPaths::ChangePathTiles( Map& theMap, const IntVec2& pathStart, const IntVec2& pathSize ) const {
    RNG* mapRNG = theMap.GetMapRNG();
    bool xFirst = mapRNG->PercentChance( 0.5f );
    IntVec2 pathPosition = pathStart;

    if( xFirst ) {
        ChangePathTilesX( theMap, pathPosition, pathSize.x );
        pathPosition += IntVec2( pathSize.x, 0 );
        ChangePathTilesY( theMap, pathPosition, pathSize.y );
    } else {
        ChangePathTilesY( theMap, pathPosition, pathSize.y );
        pathPosition += IntVec2( 0, pathSize.y );
        ChangePathTilesX( theMap, pathPosition, pathSize.x );
    }
}


void MGS_RoomsAndPaths::ChangePathTilesX( Map& theMap, const IntVec2& pathStart, int lengthX ) const {
    int offset = (lengthX > 0) ? 1 : -1; // Used to determine left vs right
    int pathLength = abs( lengthX );

    for( int stepIndex = 1; stepIndex <= pathLength; stepIndex++ ) {
        int tileX = pathStart.x + (offset * stepIndex);
        Tile& tile = GetTile( theMap, tileX, pathStart.y );
        tile.SetTileType( m_pathFloor );
    }
}


void MGS_RoomsAndPaths::ChangePathTilesY( Map& theMap, const IntVec2& pathStart, int lengthY ) const {
    int offset = (lengthY > 0) ? 1 : -1; // Used to determine left vs right
    int pathLength = abs( lengthY );

    for( int stepIndex = 1; stepIndex <= pathLength; stepIndex++ ) {
        int tileY = pathStart.y + (offset * stepIndex);
        Tile& tile = GetTile( theMap, pathStart.x, tileY );
        tile.SetTileType( m_pathFloor );
    }
}
