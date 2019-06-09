#include "Game/GameInput.hpp"

#include "Engine/Core/WindowContext.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"


GameInput::GameInput() {

}


GameInput::~GameInput() {

}


void GameInput::Startup() {

}


void GameInput::Shutdown() {

}


void GameInput::Update( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    m_toggleInventory = false;
}


bool GameInput::HandleKeyPressed( unsigned char keyCode ) {
    switch( keyCode ) {
        case( KB_ESCAPE ): { //  Escape
            //m_pause = !m_pause; // DFS1FIXME: Causing problems.. just disable until it can be fixed
            return true;
        } case( KB_W ): { // Up
            m_upPressed = true;
            return true;
        } case( KB_A ): { // Left
            m_leftPressed = true;
            return true;
        } case( KB_S ): { // Down
            m_downPressed = true;
            return true;
        } case( KB_D ): { // Right
            m_rightPressed = true;
            return true;
        }
    }

    return false;
}


bool GameInput::HandleKeyReleased( unsigned char keyCode ) {
    switch( keyCode ) {
        case( KB_W ): { // Up
            m_upPressed = false;
            return true;
        } case( KB_A ): { // Left
            m_leftPressed = false;
            return true;
        } case( KB_S ): { // Down
            m_downPressed = false;
            return true;
        } case( KB_D ): { // Right
            m_rightPressed = false;
            return true;
        } case( KB_I ): { // Inventory
            m_toggleInventory = true;
            return true;
        }
    }

    return false;
}


bool GameInput::HandleMouseButton( MouseEvent event, float scrollAmount /*= 0.f */ ) {
    /*
    switch( event ) {
        case(MOUSE_EVENT_LBUTTON_DOWN): {
            return true;
        } case(MOUSE_EVENT_LBUTTON_UP): {
            return true;
        } case(MOUSE_EVENT_RBUTTON_DOWN): {
            return true;
        } case(MOUSE_EVENT_RBUTTON_UP): {
            return true;
        } case(MOUSE_EVENT_SCROLL): {
            m_zoomAmount = scrollAmount;
            return true;
        }
    }
    */
    UNUSED( event );
    UNUSED( scrollAmount );

    return false;
}


Vec2 GameInput::GetMovementDirection() const {
    float moveDirX = (float)(-(int)m_leftPressed + (int)m_rightPressed);
    float moveDirY = (float)(-(int)m_downPressed + (int)m_upPressed);
    Vec2 moveDir = Vec2( moveDirX, moveDirY );

    return moveDir.GetNormalized();
}


bool GameInput::WasInvtoryToggled() const {
    return m_toggleInventory;
}


bool GameInput::IsPaused() const {
    return m_pause;
}
