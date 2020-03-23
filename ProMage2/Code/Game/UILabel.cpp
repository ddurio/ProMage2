#include "Game/UILabel.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/RenderContext.hpp"


UILabel::UILabel( const Vec2& alignment, const Vec2& offset, const Vec2& relativeSize, const Vec2& absoluteSize, const std::string& text ) :
    UIWidget( alignment, offset, relativeSize, absoluteSize ) {
    SetText( text );
}


UILabel::~UILabel() {

}


void UILabel::UpdateBounds( const AABB2& containerBounds ) {
    AABB2 worldBounds = GetWorldBounds();

    if( worldBounds == containerBounds ) {
        return;
    }

    UIWidget::UpdateBounds( containerBounds );

    // Refresh text verts
    std::string text = m_text;
    m_text = "";
    SetText( text );
}


void UILabel::Render() const {
    g_theRenderer->BindTexture( GetTexture() );
    g_theRenderer->DrawVertexArray( m_textVerts );
    RenderChildren();
}


void UILabel::SetText( const std::string& text ) {
    if( m_text == text  ) {
        return;
    }

    AABB2 worldBounds = GetWorldBounds();
    Vec2 dimensions = worldBounds.GetDimensions();

    if( dimensions == Vec2::ZERO ) {
        m_text = text;
        return;
    }

    m_text = text;
    m_textVerts.clear();

    BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );

    font->AddVertsForTextInBox2D( m_textVerts, worldBounds, dimensions.y, m_text, GetColor(), 1.f, ALIGN_CENTER, TEXT_DRAW_SHRINK_TO_FIT );
    SetTexture( font->GetTexturePath() );
}
