#pragma once
#include "Game/GameCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


class MGS_CellularAutomata : public MapGenStep {
    friend class EditorMapGenStep;

    public:
    MGS_CellularAutomata( const XMLElement& element, const Strings& motifHierarchy );
    ~MGS_CellularAutomata() {};

    void SaveToXml( XmlDocument& document, XMLElement& element ) const override;
    bool RecalculateMotifVars( EventArgs& args ) override;


    private:
    IntRange m_radius               = IntRange::ONE;
    float m_chancePerTile           = 1.f;

    std::string m_ifNeighborType    = "";
    Strings m_ifNeighborHasTags;
    IntRange m_ifNumNeighbors       = IntRange( 1, 8 );


    enum TileValidity {
        UNKNOWN = -1,
        VALID,
        INVALID
    };


    void RunOnce( Map& theMap ) const;

    TileValidity IsNeighborTileValid( const Tile& neighbor ) const;
    int CountMatchingNeighbors( int homeTileX, int homeTileY, Map& theMap, std::vector< TileValidity >& tileValidities ) const;

    IntRange ParseCustomIntRange( const std::string& rangeAsStr ) const;
    std::string SaveCustomIntRange( const IntRange& range ) const;
};
