#pragma once
#include "Game/GameCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


class MGS_PerlinNoise : public MapGenStep {
    friend class EditorMapGenStep;

    public:
    MGS_PerlinNoise( const XMLElement& element );

    void SaveToXml( XmlDocument& document, XMLElement& element ) const override;


    private:
    IntRange   m_gridSize           = IntRange( 10, 30 );
    IntRange   m_numOctaves         = IntRange( 1, 3 );
    FloatRange m_octavePersistence  = FloatRange( 0.4f, 0.6f );
    FloatRange m_octaveScale        = FloatRange( 1.5f, 2.5f );

    void RunOnce( Map& theMap ) const;
};
