#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/Definition.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/IntRange.hpp"

#include "map"


class Map;
class MapGenStep;


class MapDef : public Definition< Map, MapDef > {
    friend class Definition< Map, MapDef >;

    public:
    void DefineObject( Map& theMap ) const override;

    private:
    std::string m_tileFillType = "";
    std::string m_tileEdgeType = "";
    IntRange m_width = IntRange::ZERO;
    IntRange m_height = IntRange::ZERO;
    std::vector<MapGenStep*> m_mapGenSteps;


    explicit MapDef( const XMLElement& element );
    ~MapDef();

    void DefineFillAndEdge( Map& map ) const;
    void DefineFromMGS( Map& map ) const;
    void DefineFromContextTiles( Map& map ) const;
    void DefineTileColliders( Map& map ) const;

    bool IsEdgeTile( int tileX, int tileY, int mapWidth, int mapHeight ) const;
};
