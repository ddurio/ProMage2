#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/Tags.hpp"


struct Metadata {
    public:
    Tags m_tagData;
    float m_distanceField = 999999;
    float m_noiseValue = 0.0f;
};
