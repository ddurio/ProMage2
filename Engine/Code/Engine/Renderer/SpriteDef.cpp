#include "Engine/Renderer/SpriteDef.hpp"


SpriteDef::SpriteDef( const Vec2& uvAtBottomLeft, const Vec2& uvAtTopRight, const std::string& texturePath ) :
    m_uvAtBottomLeft( uvAtBottomLeft ),
    m_uvAtTopRight( uvAtTopRight ),
    m_texturePath( texturePath ) {
}


SpriteDef::SpriteDef( const std::string& text ) {
    SetFromText( text );
}


void SpriteDef::GetUVs( Vec2& out_uvAtBottomLeft, Vec2& out_uvAtTopRight ) const {
    out_uvAtBottomLeft = m_uvAtBottomLeft;
    out_uvAtTopRight = m_uvAtTopRight;
}


std::string SpriteDef::GetTexturePath() const {
    return m_texturePath;
}


std::string SpriteDef::GetAsString() const {
    std::string uvMinStr = m_uvAtBottomLeft.GetAsString();
    std::string uvMaxStr = m_uvAtTopRight.GetAsString();
    std::string spriteDefStr = Stringf( "%s %s %s", uvMinStr.c_str(), uvMaxStr.c_str(), m_texturePath.c_str() );

    return spriteDefStr;
}


void SpriteDef::SetFromText( const std::string& text ) {
    Strings splitText = SplitStringOnDelimeter( text, ' ', false );
    int numParams = (int)splitText.size();
    GUARANTEE_OR_DIE( numParams == 3, Stringf( "(SpriteDef) Invalid SetFromText string '%s'", text.c_str() ) );

    m_uvAtBottomLeft = Vec2( splitText[0] );
    m_uvAtTopRight = Vec2( splitText[1] );
    m_texturePath = splitText[2];
}
