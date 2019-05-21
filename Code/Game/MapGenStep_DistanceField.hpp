#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/XMLUtils.hpp"

#include "Game/MapGenStep.hpp"


class Map;

class MapGenStep_DistanceField : public MapGenStep {
    public:
    explicit MapGenStep_DistanceField( const XMLElement& element );

    private:
    std::string m_movementType = "";

    void RunOnce( Map& map ) const;
    void ResetDistanceField( Map& map, std::vector<IntVec2>& openTiles ) const;
    void OpenNeighbors( Map& map, std::vector<IntVec2>& openTiles, const IntVec2& tileCoords ) const;
    bool IsTileValid( const Tile& tile ) const;
    bool VectorContains( const std::vector<IntVec2>& coordVec, const IntVec2& coords ) const;

    void EchoDistanceField( Map& map ) const;
};
