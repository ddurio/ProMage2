#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/IntRange.hpp"

#include "map"


class Map;
class MapGenStep;

class MapDef {
    public:
    MapDef( const XMLElement& element );
    ~MapDef();

    static void InitializeMapDefs();
    static const MapDef* GetMapDef( std::string mapType );

    void Define( Map& map ) const;

    private:
    static std::map<std::string, MapDef*> s_mapDefs;

    std::string m_mapType = "";
    std::string m_tileFillType = "";
    std::string m_tileEdgeType = "";
    IntRange m_width = IntRange::ZERO;
    IntRange m_height = IntRange::ZERO;
    std::vector<MapGenStep*> m_mapGenSteps;


    void DefineFillAndEdge( Map& map ) const;
    void DefineFromMGS( Map& map ) const;
    void DefineFromContextTiles( Map& map ) const;
    void DefineTileColliders( Map& map ) const;
};
