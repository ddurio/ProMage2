#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/IntRange.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


class MapGenStep_Sprinkle : public MapGenStep {
    public:
    explicit MapGenStep_Sprinkle( const XMLElement& element );
    ~MapGenStep_Sprinkle() {};


    private:
    IntRange m_count = IntRange::ZERO;

    void RunOnce( Map& map ) const;
};
