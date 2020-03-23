#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Input/Keyboard.hpp"
#include "Engine/Input/XboxController.hpp"


constexpr int MAX_CONTROLLERS = 4;
class WindowContext;
enum MouseEvent : int;


class InputSystem {
    public:
	void Startup( WindowContext* windowContext = nullptr );
	void Shutdown();

	void BeginFrame();
	void EndFrame();

    bool HandleKeyPressed( unsigned char keyCode );
    bool HandleKeyReleased( unsigned char keyCode );
    bool HandleMouseButton( MouseEvent event, float scrollAmount = 0.f );

    const KeyboardAndMouse& GetKeyboardAndMouse() const;
    const XboxController& GetController( int controllerID ) const;

    private:
    KeyboardAndMouse m_keyboardAndMouse;

    XboxController m_controllers[MAX_CONTROLLERS] = {
        XboxController( 0 ),
        XboxController( 1 ),
        XboxController( 2 ),
        XboxController( 3 )
    };
};
