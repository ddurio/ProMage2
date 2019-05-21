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

    AABB2 clientBounds = g_theWindow->GetClientBounds();
    IntVec2 mousePosition = g_theWindow->GetMouseClientPosition();

    float distanceFromLeft  = abs((float)mousePosition.x - clientBounds.mins.x);
    float distanceFromRight = abs((float)mousePosition.x - clientBounds.maxs.x);
    float distanceFromBot   = abs((float)mousePosition.y - clientBounds.mins.y);
    float distanceFromTop   = abs((float)mousePosition.y - clientBounds.maxs.y);

    m_mouseLeft  = (distanceFromLeft  <= m_mouseScrollDistance);
    m_mouseRight = (distanceFromRight <= m_mouseScrollDistance);
    m_mouseDown  = (distanceFromBot   <= m_mouseScrollDistance);
    m_mouseUp    = (distanceFromTop   <= m_mouseScrollDistance);
}


bool GameInput::HandleKeyPressed( unsigned char keyCode ) {
    switch( keyCode ) {
        case(0x1B): { //  Escape
            //m_pause = !m_pause; // SD2FIXME: Causing problems.. just disable until it can be fixed
            return true;
        } case('W'): { // Up
            m_upPressed = true;
            return true;
        } case('A'): { // Left
            m_leftPressed = true;
            return true;
        } case('S'): { // Down
            m_downPressed = true;
            return true;
        } case('D'): { // Right
            m_rightPressed = true;
            return true;
        } case('Q'): { // Rotate Left
            m_rotateLeftPressed = true;
            return true;
        } case('E'): { // Rotate Right
            m_rotateRightPressed = true;
            return true;
        }
    }

    return false;
}


bool GameInput::HandleKeyReleased( unsigned char keyCode ) {
    switch( keyCode ) {
        case('W'): { // Up
            m_upPressed = false;
            return true;
        } case('A'): { // Left
            m_leftPressed = false;
            return true;
        } case('S'): { // Down
            m_downPressed = false;
            return true;
        } case('D'): { // Right
            m_rightPressed = false;
            return true;
        } case('Q'): { // Rotate Left
            m_rotateLeftPressed = false;
            return true;
        } case('E'): { // Rotate Right
            m_rotateRightPressed = false;
            return true;
        }
    }

    return false;
}


bool GameInput::HandleMouseButton( MouseEvent event, float scrollAmount /*= 0.f */ ) {
    if( event == MOUSE_EVENT_SCROLL ) {
        m_zoomAmount = scrollAmount;
        return true;
    }

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

    return false;
}


Vec2 GameInput::GetFrameScroll() const {
    float moveKbDirX = (float)(-(int)m_leftPressed + (int)m_rightPressed);
    float moveKbDirY = (float)(-(int)m_downPressed + (int)m_upPressed);
    Vec2 moveKbDir = Vec2( moveKbDirX, moveKbDirY );

    float moveMouseDirX = (float)(-(int)m_mouseLeft + (int)m_mouseRight);
    float moveMouseDirY = (float)(-(int)m_mouseDown + (int)m_mouseUp);
    Vec2 moveMouseDir = Vec2( moveMouseDirX, moveMouseDirY );

    if( moveKbDir != Vec2::ZERO ) {
        moveKbDir.Normalize();
    }

    if( moveMouseDir != Vec2::ZERO ) {
        moveMouseDir.Normalize();
    }

    Vec2 moveKb = moveKbDir * m_keyBoardScrollSpeed;
    Vec2 moveMouse = moveMouseDir * m_mouseScrollSpeed;
    Vec2 movement = moveKb + moveMouse;

    return movement;
}


float GameInput::GetFrameZoom() {
    float zoom =  m_zoomAmount * m_zoomSpeed;
    m_zoomAmount = 0.f;
    return zoom;
}


float GameInput::GetFrameRotation() const {
    int rotationDirection = -(int)m_rotateLeftPressed + (int)m_rotateRightPressed;
    return (float)rotationDirection * m_rotationSpeed;
}


bool GameInput::IsPaused() const {
    return m_pause;
}
