#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/Definition.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"

#include "map"


class Texture;
class Tile;
class SpriteSheet;


class TileDef : public Definition< Tile, TileDef > {
    friend class Definition< Tile, TileDef >;

    public:
    struct CompareDrawOrder { // Used for priority queue based on drawOrder
        bool operator()( const TileDef* const& tileDefA, const TileDef* const& tileDefB );
    };

    void DefineObject( Tile& theObject ) const override;

    static const SpriteSheet GetTerrainSprites();
    static const TileDef* GetTileDefFromTexelColor( const Rgba& texelColor );
    static std::string GetTileTypeFromTexelColor( const Rgba& texelColor );

    const std::string& GetTileType() const;
    const std::string& GetTileContext() const;
    void GetUVs( Vec2& uvMins, Vec2& uvMaxs) const;
    const Rgba& GetSpriteTint() const;
    const Rgba& GetTexelColor() const;
    int GetDrawOrder() const;
    const Strings& GetExtraRenderTypes() const;

    bool AllowsSight() const;
    bool AllowsWalking() const;
    bool AllowsFlying() const;
    bool AllowsSwimming() const;


    private:
    static int s_numTileTypes;
    static const SpriteSheet* s_terrainSprites;

    int m_drawOrder = -1;
    std::string m_tileContext = "single";
    Strings m_extraRenderTypes;

    IntVec2 m_spriteCoords = IntVec2::ZERO;
    AABB2 m_uvCoords = AABB2();

    Rgba m_spriteTint = Rgba::WHITE;
    Rgba m_texelColor = Rgba( 0.f, 0.f, 0.f, 0.f ); // Transparent, will never match a TexelColor

    bool m_allowsSight = false;
    bool m_allowsWalking = false;
    bool m_allowsFlying = false;
    bool m_allowsSwimming = false;


    explicit TileDef( const XMLElement& element );
    explicit TileDef( const TileDef& defToCopy, const IntVec2& offsetCoords ); // Used for edged tiles

    void DefineEdgeTileDefs();
};
