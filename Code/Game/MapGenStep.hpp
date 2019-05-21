#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/FloatRange.hpp"


class Map;
class Tile;
class TileDef;

class MapGenStep {
    public:
    explicit MapGenStep( const XMLElement& element );
    ~MapGenStep() {};

    static MapGenStep* CreateMapGenStep( const XMLElement& element );

    void Run( Map& map ) const;

    virtual bool IsTileValid( const Tile& tile ) const;
    virtual void ChangeTile( Map& map, int tileIndex ) const;
    virtual void ChangeTile( Map& map, int tileX, int tileY ) const;

    Tile& GetTile( Map& map, int tileIndex ) const;
    Tile& GetTile( Map& map, int tileX, int tileY ) const;

    protected:
    std::string m_stepType        = "";

    // Conditions
    std::string m_ifIsType        = "";
    std::string m_ifHasTags       = "";
    FloatRange  m_ifDistanceField = FloatRange::NEGONE;
    FloatRange  m_ifNoise         = FloatRange::NEGONE;

    // Results
    std::string m_changeToType    = "";
    std::string m_setTags         = "";
    FloatRange m_setDistanceField = FloatRange::NEGONE;
    std::string m_spawnActorType  = "";
    std::string m_spawnItemType   = "";

    float m_chanceToRun = 1.f;
    IntRange m_numIterations = IntRange::ONE;

    virtual void RunOnce( Map& map ) const = 0;
};
