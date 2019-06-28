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

    m_toggleInventory   = false;
    m_interact          = false;
    m_escapePressed     = false;
    m_attack            = false;
}


bool GameInput::HandleKeyPressed( unsigned char keyCode ) {
    switch( keyCode ) {
        case( KB_ESCAPE ): { //  Escape
            m_escapePressed = true;
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
        } case( KB_I ): { // Inventory
            m_toggleInventory = true;
            return true;
        } case( KB_F ): {     // Interact (intentional fall-through)
        } case( KB_SPACE ): { // Interact
            m_interact = true;
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
        }
    }

    return false;
}


bool GameInput::HandleMouseButton( MouseEvent event, float scrollAmount /*= 0.f */ ) {
    UNUSED( scrollAmount );

    switch( event ) {
        case(MOUSE_EVENT_LBUTTON_DOWN): {
            m_attack = true;
            return true;
            /*
        } case(MOUSE_EVENT_LBUTTON_UP): {
            return true;
        } case(MOUSE_EVENT_RBUTTON_DOWN): {
            return true;
        } case(MOUSE_EVENT_RBUTTON_UP): {
            return true;
        } case(MOUSE_EVENT_SCROLL): {
            m_zoomAmount = scrollAmount;
            return true;
            */
        }
    }

    return false;
}


Vec2 GameInput::GetMovementDirection() const {
    float moveDirX = (float)(-(int)m_leftPressed + (int)m_rightPressed);
    float moveDirY = (float)(-(int)m_downPressed + (int)m_upPressed);
    Vec2 moveDir = Vec2( moveDirX, moveDirY );

    return moveDir.GetNormalized();
}


bool GameInput::WasInventoryToggled() const {
    return m_toggleInventory;
}


bool GameInput::ShouldInteract() const {
    return m_interact;
}


bool GameInput::ShouldAttack() const {
    return m_attack;
}


bool GameInput::ShouldExitMenu() const {
    return m_escapePressed;
}


bool GameInput::IsPaused() const {
    return m_pause;
}
