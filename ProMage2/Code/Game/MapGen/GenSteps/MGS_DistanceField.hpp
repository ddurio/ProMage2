#pragma once
#include "Game/GameCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


class MGS_DistanceField : public MapGenStep {
    friend class EditorMapGenStep;

    public:
    explicit MGS_DistanceField( const XMLElement& element, const Strings& motifHierarchy );
    explicit MGS_DistanceField( const Strings& motifHierarchy );

    void SaveToXml( XmlDocument& document, XMLElement& element ) const override;
    bool RecalculateMotifVars( EventArgs& args ) override;


    private:
    static constexpr float INVALID_DISTANCE = 999999.f;

    std::string m_heatMapName = "Distance";
    std::string m_movementType = "Walk";
    int m_maxDistance = (int)INVALID_DISTANCE;

    void RunOnce( Map& theMap ) const;

    void SetupDistanceField( Map& map, std::vector<IntVec2>& openTiles ) const;
    void OpenNeighbors( Map& map, std::vector<IntVec2>& openTiles, std::vector<bool>& openedTiles, const IntVec2& tileCoords ) const;

    bool IsNeighborTileValid( const Tile& tile ) const;
    void ChangeTileDistance( Map& theMap, int tileIndex, float distance ) const;

    static Strings GetMovementTypes();

    //void EchoDistanceField( Map& map ) const;
};
