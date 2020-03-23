#pragma once
#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Input/KeyButtonState.hpp"


enum XboxButtonID {
    XBOX_BUTTON_ID_INVALID = -1,
    XBOX_BUTTON_ID_A,
    XBOX_BUTTON_ID_B,
    XBOX_BUTTON_ID_X,
    XBOX_BUTTON_ID_Y,

    XBOX_BUTTON_ID_DUP,
    XBOX_BUTTON_ID_DDOWN,
    XBOX_BUTTON_ID_DLEFT,
    XBOX_BUTTON_ID_DRIGHT,

    XBOX_BUTTON_ID_SELECT,
    XBOX_BUTTON_ID_START,

    XBOX_BUTTON_ID_LBUMPER,
    XBOX_BUTTON_ID_RBUMPER,

    XBOX_BUTTON_ID_LSTICK,
    XBOX_BUTTON_ID_RSTICK,

    XBOX_BUTTON_ID_LTRIGGER,
    XBOX_BUTTON_ID_RTRIGGER,

    NUM_XBOX_BUTTON_IDS
};


class XboxController {
    public:
    explicit XboxController( int controllerID );

	void Startup();
	void Shutdown();

    void UpdateInput();
    void Reset();

    bool IsConnected() const;
    int GetControllerID() const;
    const AnalogJoystick& GetLeftJoystick() const;
    const AnalogJoystick& GetRightJoystick() const;
    float GetLeftTrigger() const;
    float GetRightTrigger() const;
    const KeyButtonState& GetKeyButtonState( XboxButtonID keyCode ) const;

    private:
    const int m_controllerID = -1;
    bool m_isConnected = false;
    AnalogJoystick m_leftJoystick = AnalogJoystick( 0.3f, 0.9f );
    AnalogJoystick m_rightJoystick = AnalogJoystick( 0.3f, 0.9f );
    KeyButtonState m_buttons[NUM_XBOX_BUTTON_IDS]{};
    float m_leftTrigger = 0.f;
    float m_rightTrigger = 0.f;
};
