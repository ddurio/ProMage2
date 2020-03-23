#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Vertex/VertexUtils.hpp"

#include "vector"


enum TextDrawMode {
    TEXT_DRAW_OVERRUN,
    TEXT_DRAW_SHRINK_TO_FIT
};

struct AABB2;
class SpriteSheet;
class Texture2D;
struct Vec2;
struct VertexPCU;

class BitmapFont {
    friend class RenderContext;

    private:
    // Only RenderContext can make a new font
    explicit BitmapFont( const std::string& fontName, const std::string& texturePath );

    public:
    std::string GetTexturePath() const;

    void AddVertsForText2D( VertexList& textVerts, const Vec2& textPositionMins, float cellHeight, const std::string& text, const Rgba& tint = Rgba::WHITE, float cellAspect = 1.f, int maxGlyphs = 9999999 ) const;
    void AddVertsForTextInBox2D( VertexList& textVerts, const AABB2& boxBounds, float cellHeight, const std::string& text, const Rgba& tint = Rgba::WHITE, float cellAspect = 1.f, const Vec2& alignment = ALIGN_CENTER, TextDrawMode mode = TEXT_DRAW_OVERRUN, int maxGlyphs = 9999999 ) const;

    float GetGlyphAspect( int glyphUnicode ) const;

    private:
    std::string m_name = "";
    SpriteSheet* m_glyphSpriteSheet = nullptr;

    Vec2 GetTextSize( float cellHeight, const std::string& text, float cellAspect ) const;
    Vec2 GetTextSize( float cellHeight, const std::vector<std::string>& text, float cellAspect ) const;
    float CalculateShrinkToFitScale( const Vec2& textDimensions, const Vec2& boundsDimensions ) const;
};
