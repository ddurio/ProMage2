#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Core/Time.hpp"


constexpr float KEYBUTTONSTATE_ANALOG_THRESHOLD_UP = 0.3f;
constexpr float KEYBUTTONSTATE_ANALOG_THRESHOLD_DOWN = 0.7f;


KeyButtonState::KeyButtonState() {

}


KeyButtonState::~KeyButtonState() {

}


bool KeyButtonState::IsPressed() const {
    return m_isPressed;
}


bool KeyButtonState::WasJustPressed() const {
    return (m_isPressed && !m_wasPressedLastFrame);
}


bool KeyButtonState::WasJustReleased() const {
    return (!m_isPressed && m_wasPressedLastFrame);
}


void KeyButtonState::SetState( bool newState ) {
    m_wasPressedLastFrame = m_isPressed;
    m_isPressed = newState;
}


void KeyButtonState::UpdateSimpleButton( unsigned short buttonStates, unsigned short bitmaskFlag ) {
    m_wasPressedLastFrame = m_isPressed;
    m_isPressed = (buttonStates & bitmaskFlag) != 0;
}


void KeyButtonState::UpdateAnalogButton( float pressedValue ) {
    m_wasPressedLastFrame = m_isPressed;

    if( pressedValue > KEYBUTTONSTATE_ANALOG_THRESHOLD_DOWN ) {
        m_isPressed = true;
    } else if( pressedValue < KEYBUTTONSTATE_ANALOG_THRESHOLD_UP ) {
        m_isPressed = false;
    }
}


void KeyButtonState::Reset() {
    m_wasPressedLastFrame = false;
    m_isPressed = false;
}
