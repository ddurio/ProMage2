#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"


class SpriteDef {
    public:
    explicit SpriteDef( const Vec2& uvAtBottomLeft, const Vec2& uvAtTopRight, const std::string& texturePath );
    explicit SpriteDef( const std::string& text );

    void GetUVs( Vec2& out_uvAtBottomLeft, Vec2& out_uvAtTopRight ) const;
    std::string GetTexturePath() const;

    std::string GetAsString() const;
    void SetFromText( const std::string& text );

    private:
    Vec2 m_uvAtBottomLeft = Vec2::ZERO;
    Vec2 m_uvAtTopRight = Vec2::ONE;
    std::string m_texturePath = "";
};