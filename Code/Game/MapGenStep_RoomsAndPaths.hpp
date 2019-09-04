#pragma once
#include "Game/GameCommon.hpp"

#include "MapGen/GenSteps/MapGenStep.hpp"


class MapGenStep_RoomsAndPaths : public MapGenStep {
    public:
    MapGenStep_RoomsAndPaths( const XMLElement& element );

    private:
    // Rooms
    IntRange    m_numRooms           = IntRange::ZERO;
    IntRange    m_roomWidth          = IntRange::ZERO;
    IntRange    m_roomHeight         = IntRange::ZERO;
    std::string m_roomFloor          = "";
    std::string m_roomWall           = "";
    IntRange    m_numOverlaps        = IntRange::ZERO;

    // Paths
    std::string m_pathFloor          = "";
    bool        m_pathLoop           = true;
    IntRange    m_numExtraPaths      = IntRange::ZERO;
    FloatRange  m_pathStraightChance = FloatRange::ZERO;

    void RunOnce( Map& map ) const;

    bool GenerateRooms( const Map& map, std::vector<IntVec2>& roomPositions, std::vector<IntVec2>& roomSizes ) const;
    bool AddRoomIfValid( std::vector<IntVec2>& positions, std::vector<IntVec2>& sizes, int& remainingOverlaps, const IntVec2& newPosition, const IntVec2& newSize ) const;
    bool DoRoomsOverlap( const IntVec2& roomPositionA, const IntVec2& roomSizeA, const IntVec2& roomPositionB, const IntVec2& roomSizeB ) const;
    void ChangeRoomTiles( Map& map, const std::vector<IntVec2>& roomPositions, std::vector<IntVec2>& roomSizes ) const;

    void GetRoomCenters( const std::vector<IntVec2>& positions, const std::vector<IntVec2>& sizes, std::vector<IntVec2>& centers ) const;
    void GeneratePaths( Map& map, std::vector<IntVec2>& roomCenters ) const;
    void ChangePathTiles( Map& map, const IntVec2& pathStart, const IntVec2& pathSize ) const;
    void ChangePathTilesX( Map& map, const IntVec2& pathStart, int lengthX ) const;
    void ChangePathTilesY( Map& map, const IntVec2& pathStart, int lengthY ) const;
};
