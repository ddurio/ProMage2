#pragma once
#include "Game/GameCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


class MGS_DistanceField : public MapGenStep {
    public:
    explicit MGS_DistanceField( const XMLElement& element );

    private:
    std::string m_movementType = "";

    void RunOnce( Map& theMap ) const;

    void SetupDistanceField( Map& map, std::vector<IntVec2>& openTiles ) const;
    void OpenNeighbors( Map& map, std::vector<IntVec2>& openTiles, std::vector<bool>& openedTiles, const IntVec2& tileCoords ) const;

    bool IsNeighborTileValid( const Tile& tile ) const;

    //void EchoDistanceField( Map& map ) const;
};
