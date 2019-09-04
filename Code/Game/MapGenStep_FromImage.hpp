#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/FloatRange.hpp"

#include "MapGen/GenSteps/MapGenStep.hpp"


class Image;
class Map;

class MapGenStep_FromImage : public MapGenStep {
    public:
    MapGenStep_FromImage( const XMLElement& element );
    ~MapGenStep_FromImage();

    private:
    Image* m_image = nullptr;
    FloatRange m_alignX = FloatRange::ZEROTOONE;
    FloatRange m_alignY = FloatRange::ZEROTOONE;
    IntRange m_numRotations = IntRange::ZERO;

    void RunOnce( Map& map ) const;
};
