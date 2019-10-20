#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/IntRange.hpp"
#include "Engine/Utils/Definition.hpp"
#include "Engine/Utils/XMLUtils.hpp"

#include "map"


class Map;
class MapGenStep;
class RNG;
class Tile;


class MapDef : public Definition< Map, MapDef > {
    friend class Definition< Map, MapDef >;

    public:
    virtual void DefineObject( Map& theMap ) const override;

    virtual void SaveToXml( XmlDocument& document, XMLElement& element ) const;


    protected:
    std::string m_motif = "";

    std::string m_tileFillType = "";
    std::string m_tileEdgeType = "";
    IntRange m_width = IntRange::ZERO;
    IntRange m_height = IntRange::ZERO;
    std::vector<MapGenStep*> m_mapGenSteps;


    explicit MapDef( const XMLElement& element );
    ~MapDef();

    RNG* GetMapRNG( const Map& theMap ) const;
    Tile& GetTile( Map& theMap, int tileIndex ) const;

    void SetMapDimensions( Map& theMap, const IntVec2& dimensions ) const;
    void SetMapDef( Map& theMap ) const;

    void DefineFillAndEdge( Map& theMap ) const;
    void DefineFromMGS( Map& theMap ) const;
    virtual void DefineFromContextTiles( Map& theMap ) const;
    void DefineTileColliders( Map& theMap ) const;

    bool IsEdgeTile( int tileX, int tileY, int mapWidth, int mapHeight ) const;
};
