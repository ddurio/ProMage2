#include "Engine/Input/InputSystem.hpp"

#include "Engine/Core/WindowContext.hpp"
#include "Engine/Debug/Profiler.hpp"


void InputSystem::Startup( WindowContext* windowContext /*= nullptr */ ) {
    if( windowContext != nullptr ) {
        windowContext->RegisterInputSystem( this );
    }
}


void InputSystem::Shutdown() {

}


void InputSystem::BeginFrame() {
    PROFILE_FUNCTION();

    m_keyboardAndMouse.BeginFrame();

    for( int i = 0; i < MAX_CONTROLLERS; i++ ) {
        m_controllers[i].UpdateInput();
    }
}


void InputSystem::EndFrame() {
    m_keyboardAndMouse.EndFrame();
}


bool InputSystem::HandleKeyPressed( unsigned char keyCode ) {
    return m_keyboardAndMouse.HandleKeyPressed( keyCode );
}


bool InputSystem::HandleKeyReleased( unsigned char keyCode ) {
    return m_keyboardAndMouse.HandleKeyReleased( keyCode );
}


bool InputSystem::HandleMouseButton( MouseEvent event, float scrollAmount /*= 0.f */ ) {
    return m_keyboardAndMouse.HandleMouseButton( event, scrollAmount );
}


const KeyboardAndMouse& InputSystem::GetKeyboardAndMouse() const {
    return m_keyboardAndMouse;
}


const XboxController& InputSystem::GetController( int controllerID ) const {
    return m_controllers[controllerID];
}
