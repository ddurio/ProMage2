#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/Tags.hpp"

#include "Game/TileDef.hpp"

#include "queue"


typedef std::priority_queue< const TileDef*, std::vector< const TileDef* >, TileDef::CompareDrawOrder > TileQueue;


struct Metadata {
    public:
    Tags m_tagData;
    float m_distanceField = 999999;
    float m_noiseValue = 0.0f;
    TileQueue m_renderTypes;
};
