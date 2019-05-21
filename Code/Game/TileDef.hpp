#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "map"


class Texture;
class SpriteSheet;

class TileDef {
    public:
    explicit TileDef( const XMLElement& element );

    static void InitializeTileDefs();
    static void DestroyTileDefs();
    static const TileDef* GetTileDef( std::string tileType );
    static const TileDef* GetTileDefFromTexelColor( const Rgba& texelColor );
    static std::string GetTileTypeFromTexelColor( const Rgba& texelColor );
    static std::string GetSpriteTexture();
    static const SpriteSheet GetSpriteSheet();

    const std::string& GetTileType() const;
    void GetUVs( Vec2& uvMins, Vec2& uvMaxs) const;
    const Rgba& GetSpriteTint() const;
    const Rgba& GetTexelColor() const;

    bool AllowsSight() const;
    bool AllowsWalking() const;
    bool AllowsFlying() const;
    bool AllowsSwimming() const;

    private:
    static std::map<std::string, TileDef*> s_tileDefs;
    static SpriteSheet* s_terrainSprites;

    std::string m_tileType = "";
    AABB2 m_uvCoords = AABB2();
    Rgba m_spriteTint = Rgba::WHITE;
    Rgba m_texelColor = Rgba( 0.f, 0.f, 0.f, 0.f ); // Transparent, will never match a TexelColor
    bool m_allowsSight = false;
    bool m_allowsWalking = false;
    bool m_allowsFlying = false;
    bool m_allowsSwimming = false;
};
