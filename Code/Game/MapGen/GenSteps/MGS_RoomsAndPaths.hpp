#pragma once
#include "Game/GameCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


class MGS_RoomsAndPaths : public MapGenStep {
    friend class EditorMapGenStep;

    public:
    explicit MGS_RoomsAndPaths( const XMLElement& element, const Strings& motifHierarchy );
    explicit MGS_RoomsAndPaths() {};

    void SaveToXml( XmlDocument& document, XMLElement& element ) const override;
    bool RecalculateMotifVars( EventArgs& args ) override;


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

    void RunOnce( Map& theMap ) const;

    // Rooms
    void GenerateRooms( Map& theMap, std::vector<IntVec2>& roomPositions, std::vector<IntVec2>& roomSizes ) const;
    bool AddRoomIfValid( Map& theMap, std::vector<IntVec2>& positions, std::vector<IntVec2>& sizes, int& remainingOverlaps, const IntVec2& newPosition, const IntVec2& newSize ) const;
    bool DoRoomsOverlap( const IntVec2& roomPositionA, const IntVec2& roomSizeA, const IntVec2& roomPositionB, const IntVec2& roomSizeB ) const;
    bool IsAlignmentValid( Map& theMap, const IntVec2& roomPosition, const IntVec2& roomSize ) const;
    void ChangeRoomTiles( Map& theMap, const std::vector<IntVec2>& roomPositions, std::vector<IntVec2>& roomSizes ) const;

    // Paths
    void GetRoomCenters( const std::vector<IntVec2>& positions, const std::vector<IntVec2>& sizes, std::vector<IntVec2>& centers ) const;
    void GeneratePaths( Map& theMap, std::vector<IntVec2>& roomCenters ) const;
    void ChangePathTiles( Map& theMap, const IntVec2& pathStart, const IntVec2& pathSize ) const;
    void ChangePathTilesX( Map& theMap, const IntVec2& pathStart, int lengthX ) const;
    void ChangePathTilesY( Map& theMap, const IntVec2& pathStart, int lengthY ) const;
};
