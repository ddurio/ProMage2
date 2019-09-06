#pragma once
#include "Game/GameCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


class Map;

class MapGenStep_PerlinNoise : public MapGenStep {
    public:
    MapGenStep_PerlinNoise( const XMLElement& element );

    private:
    FloatRange m_ifNoise = FloatRange::NEGONE;
    IntRange m_numOctaves = IntRange( 1, 3 );
    IntRange m_gridSize = IntRange( 10, 30 );
    FloatRange m_octavePersistence = FloatRange( 0.4f, 0.6f );
    FloatRange m_octaveScale = FloatRange( 1.5f, 2.5f );
    //int m_seed = 0;

    void RunOnce( Map& map ) const;
};
