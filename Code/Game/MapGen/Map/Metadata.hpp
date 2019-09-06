#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/Tags.hpp"

#include "Game/MapGen/Map/TileDef.hpp"

#include "queue"


typedef std::priority_queue< const TileDef*, std::vector< const TileDef* >, TileDef::CompareDrawOrder > TileQueue;


struct Metadata {
    public:
    Tags m_tagData;
    std::map< std::string, float, StringCmpCaseI > m_heatMaps;
    TileQueue m_renderTypes;


    bool IsHeatMapSet( const std::string& heatMapName ) const;
};
