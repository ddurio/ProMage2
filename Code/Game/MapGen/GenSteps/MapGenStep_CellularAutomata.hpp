#pragma once
#include "Game/GameCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


class Map;

class MapGenStep_CellularAutomata : public MapGenStep {
    public:
    MapGenStep_CellularAutomata( const XMLElement& element );
    ~MapGenStep_CellularAutomata() {};

    private:
    std::string m_ifNeighborType    = "";
    std::string m_ifNeighborHasTags = "";

    IntRange m_ifNumNeighbors = IntRange( 1, 8 );

    int m_radius          = 1;
    float m_chancePerTile = 1.f;

    void RunOnce( Map& map ) const;

    bool IsNeighborTileValid( const Tile& neighbor ) const;
};
