#include "Engine/Input/Keyboard.hpp"

#include "Engine/Core/WindowContext.hpp"


void KeyboardAndMouse::BeginFrame() {

}


void KeyboardAndMouse::EndFrame() {
    // Effectively moves frame counter forward for each button
    for( int keyIndex = 0; keyIndex < NUM_KEYBOARD_KEYS; keyIndex++ ) {
        KeyButtonState& button = m_keyboardStates[keyIndex];

        bool isPressed = button.IsPressed();
        button.SetState( isPressed );
    }

    for( int keyIndex = 0; keyIndex < NUM_MOUSE_BUTTONS; keyIndex++ ) {
        KeyButtonState& button = m_mouseStates[keyIndex];

        bool isPressed = button.IsPressed();
        button.SetState( isPressed );
    }

    m_mouseScroll = 0.f;
}


bool KeyboardAndMouse::HandleKeyPressed( unsigned char keyCode ) {
    KeyButtonState& button = m_keyboardStates[keyCode];
    button.SetState( true );
    
    return true;
}


bool KeyboardAndMouse::HandleKeyReleased( unsigned char keyCode ) {
    KeyButtonState& button = m_keyboardStates[keyCode];
    button.SetState( false );

    return true;
}


bool KeyboardAndMouse::HandleMouseButton( MouseEvent event, float scrollAmount /*= 0.f */ ) {
    // Stored in custom order: Left (0), Right (1), Middle (2)
    KeyButtonState* buttonPtr = nullptr;
    bool buttonPressed = true;

    switch( event ) {
        case(MOUSE_EVENT_LBUTTON_UP): {     // Left mouse button
            buttonPressed = false;      // Intentional fall-through
        } case(MOUSE_EVENT_LBUTTON_DOWN): {
            buttonPtr = &m_mouseStates[0];
            break;

        } case(MOUSE_EVENT_RBUTTON_UP): {   // Right mouse button
            buttonPressed = false;      // Intentional fall-through
        } case(MOUSE_EVENT_RBUTTON_DOWN): {
            buttonPtr = &m_mouseStates[1];
            break;

        } case(MOUSE_EVENT_MBUTTON_UP): {   // Middle mouse button
            buttonPressed = false;      // Intentional fall-through
        } case(MOUSE_EVENT_MBUTTON_DOWN): {
            buttonPtr = &m_mouseStates[2];
            break;
        } case(MOUSE_EVENT_SCROLL): {
            m_mouseScroll += scrollAmount;
            return true;
        }
    }

    // Update the button
    buttonPtr->SetState( buttonPressed );
    return true;
}


const KeyButtonState& KeyboardAndMouse::GetKeyButtonState( unsigned char keyCode ) const {
    return m_keyboardStates[keyCode];
}


const KeyButtonState& KeyboardAndMouse::GetMouseButtonState( MouseButton buttonCode ) const {
    return m_mouseStates[buttonCode];
}
