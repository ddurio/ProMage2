#pragma once
#include "Game/GameCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


class Map;


class MGS_CellularAutomata : public MapGenStep {
    public:
    MGS_CellularAutomata( const XMLElement& element );
    ~MGS_CellularAutomata() {};


    private:
    IntRange m_radius               = IntRange::ONE;
    float m_chancePerTile           = 1.f;

    std::string m_ifNeighborType    = "";
    std::string m_ifNeighborHasTags = "";
    IntRange m_ifNumNeighbors       = IntRange( 1, 8 );

    enum TileValidity {
        UNKNOWN = -1,
        VALID,
        INVALID
    };


    void RunOnce( Map& theMap ) const;

    //void ValidateTile( const Tile& tile, TileValidity& tileValidity ) const;
    TileValidity IsNeighborTileValid( const Tile& neighbor ) const;

    int CountMatchingNeighbors( int homeTileX, int homeTileY, Map& theMap, std::vector< TileValidity >& tileValidities ) const;

    IntRange ParseCustomIntRange( const std::string& rangeAsStr ) const;
};
