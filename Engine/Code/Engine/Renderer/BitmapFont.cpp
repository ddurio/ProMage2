#include "Engine/Renderer/BitmapFont.hpp"

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture2D.hpp"
#include "Engine/Vertex/VertexPCU.hpp"


BitmapFont::BitmapFont( const std::string& fontName, const std::string& texturePath ) :
    m_name(fontName) {
    m_glyphSpriteSheet = new SpriteSheet( texturePath, IntVec2( 16, 16 ) );
}


std::string BitmapFont::GetTexturePath() const {
    return m_glyphSpriteSheet->GetTexturePath();
}


void BitmapFont::AddVertsForText2D( VertexList& textVerts, const Vec2& textPositionMins, float cellHeight, const std::string& text, const Rgba& tint /*= Rgba::WHITE*/, float cellAspect /*= 1.f */, int maxGlyphs /*= 9999999*/ ) const {
    float cellWidth = cellAspect * cellHeight;

    Vec2 letterMins = textPositionMins;
    Vec2 letterMaxs( letterMins.x, letterMins.y + cellHeight );
    float letterWidth;

    for( int letterIndex = 0; letterIndex < text.length() && letterIndex < maxGlyphs; letterIndex++ ) {
        char letter = text[letterIndex];
        letterWidth = cellWidth * GetGlyphAspect( letter );
        letterMaxs.x += letterWidth;

        AABB2 letterBounds( letterMins, letterMaxs );
        SpriteDef letterSprite = m_glyphSpriteSheet->GetSpriteDef( letter );
        Vec2 uvMins;
        Vec2 uvMaxs;
        letterSprite.GetUVs( uvMins, uvMaxs );

        AddVertsForAABB2D( textVerts, letterBounds, tint, uvMins, uvMaxs );

        letterMins.x += letterWidth;
    }
}


void BitmapFont::AddVertsForTextInBox2D( VertexList& textVerts, const AABB2& boxBounds, float cellHeight, const std::string& text, const Rgba& tint /*= Rgba::WHITE*/, float cellAspect /*= 1.f */,  const Vec2& alignment /*= ALIGN_CENTER*/, TextDrawMode mode /*= ALIGN_OVERRUN*/,int maxGlyphs /*= 9999999*/ ) const {
    std::vector<std::string> splitString = SplitStringOnDelimeter( text, '\n' );

    Vec2 textDimensions = GetTextSize( cellHeight, splitString, cellAspect );
    Vec2 boxDimensions = boxBounds.GetDimensions();

    if( mode == TEXT_DRAW_SHRINK_TO_FIT ) {
        float scale = CalculateShrinkToFitScale( textDimensions, boxDimensions );
        cellHeight *= scale;
        textDimensions *= scale;
    }

    AABB2 textBounds = boxBounds.GetBoxWithin( textDimensions, alignment );
    int numStrings = (int)splitString.size();
    int glyphsRemaining = maxGlyphs;

    for( int stringIndex = 0; stringIndex < numStrings; stringIndex++ ) {
        std::string lineText = splitString[stringIndex];

        AABB2 lineBounds = textBounds.CarveBoxOffTop( 0.f, cellHeight );
        Vec2 lineDimensions = GetTextSize( cellHeight, lineText, cellAspect );
        AABB2 alignedBounds = lineBounds.GetBoxWithin( lineDimensions, alignment );

        AddVertsForText2D( textVerts, alignedBounds.mins, cellHeight, lineText, tint, cellAspect, glyphsRemaining );
        glyphsRemaining -= (int)lineText.length();
    }
}


float BitmapFont::GetGlyphAspect( int glyphUnicode ) const {
    UNUSED( glyphUnicode );
    return 1.f;
}


Vec2 BitmapFont::GetTextSize( float cellHeight, const std::string& text, float cellAspect ) const {
    float cellWidth = cellAspect * cellHeight;
    float length = 0.0f;

    for( char letter : text ) {
        length += cellWidth * GetGlyphAspect( letter );
    }

    length *= cellAspect;

    return Vec2( length, cellHeight );
}


Vec2 BitmapFont::GetTextSize( float cellHeight, const std::vector<std::string>& splitString, float cellAspect ) const {
    float cellWidth = cellAspect * cellHeight;
    float maxLength = -9999999.f;

    int numStrings = (int)splitString.size();
    for( int stringIndex = 0; stringIndex < numStrings; stringIndex++ ) {
        float length = 0.0f;
        std::string text = splitString[stringIndex];

        for( char letter : text ) {
            length += cellWidth * GetGlyphAspect( letter );
        }

        length *= cellAspect;

        if( length > maxLength ) {
            maxLength = length;
        }
    }

    return Vec2( maxLength, cellHeight * numStrings );
}


float BitmapFont::CalculateShrinkToFitScale( const Vec2& textDimensions, const Vec2& boundsDimensions ) const {
    float scaleX = 1.f;
    float scaleY = 1.f;

    float textAbsX = fabsf( textDimensions.x );
    float textAbsY = fabsf( textDimensions.y );
    float boxAbsX = fabsf( boundsDimensions.x );
    float boxAbsY = fabsf( boundsDimensions.y );

    if( textAbsX > boxAbsX ) {
        scaleX = boxAbsX / textAbsX;
    }

    if( textAbsY > boxAbsY ) {
        scaleY = boxAbsY / textAbsY;
    }

    float scale = fmin( scaleX, scaleY );
    return scale;
}

