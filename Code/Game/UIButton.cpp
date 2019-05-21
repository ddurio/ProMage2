#include "Game/UIButton.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Renderer/ColorTargetView.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/Game.hpp"


UIButton::UIButton( const Vec2& alignment, const Vec2& offset, const Vec2& relativeSize, const Vec2& absoluteSize, const std::string& commandString ) :
    UIWidget( alignment, offset, relativeSize, absoluteSize ) {
    SetCommandString( commandString );
}


UIButton::~UIButton() {

}


bool UIButton::HandleMouseButton( MouseEvent event, float scrollAmount /*= 0.f */ ) {
    bool handled =  HandleMouseButtonChildren( event, scrollAmount );

    if( handled ) {
        return true;
    } else if( event == MOUSE_EVENT_LBUTTON_UP ) {
        IntVec2 mouseClient = g_theWindow->GetMouseClientPosition();
        IntVec2 ctvDimensions = g_theRenderer->GetBackBufferDimensions();
        Camera* uiCamera = g_theGame->GetUICamera();

        Vec3 mouseWorld = uiCamera->ClientToWorldPoint( mouseClient, 0.f, ctvDimensions );
        AABB2 worldBounds = GetWorldBounds();

        if( worldBounds.IsPointInside( Vec2( mouseWorld.x, mouseWorld.y ) ) ) {
            OnClick();
            return true;
        }
    }

    return false;
}


void UIButton::OnClick() {
    if( m_commandString != "" ) {
        g_theDevConsole->ExecuteCommandString( m_commandString );
    }
}


void UIButton::SetCommandString( const std::string& commandString ) {
    m_commandString = commandString;
}
