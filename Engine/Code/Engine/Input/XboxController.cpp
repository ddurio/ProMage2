#include "Engine/Input/XboxController.hpp"
#include "Engine/Math/MathUtils.hpp"
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "Xinput.h"
#pragma comment( lib, "xinput9_1_0" )


XboxController::XboxController( int controllerID )
    : m_controllerID( controllerID ) {
}


void XboxController::Startup() {

}


void XboxController::Shutdown() {

}


void XboxController::UpdateInput() {
    XINPUT_STATE xboxControllerState;
    memset( &xboxControllerState, 0, sizeof( xboxControllerState ) );
    DWORD errorStatus = XInputGetState( m_controllerID, &xboxControllerState );

    // Check connection status
    if( errorStatus != ERROR_SUCCESS ) {
        m_isConnected = false;
        Reset();
        return;
    }

    m_isConnected = true;
    unsigned short keyStates = xboxControllerState.Gamepad.wButtons;

    // Update Buttons
    m_buttons[XBOX_BUTTON_ID_A].UpdateSimpleButton( keyStates, XINPUT_GAMEPAD_A );
    m_buttons[XBOX_BUTTON_ID_B].UpdateSimpleButton( keyStates, XINPUT_GAMEPAD_B );
    m_buttons[XBOX_BUTTON_ID_X].UpdateSimpleButton( keyStates, XINPUT_GAMEPAD_X );
    m_buttons[XBOX_BUTTON_ID_Y].UpdateSimpleButton( keyStates, XINPUT_GAMEPAD_Y );
    m_buttons[XBOX_BUTTON_ID_DUP].UpdateSimpleButton( keyStates, XINPUT_GAMEPAD_DPAD_UP );
    m_buttons[XBOX_BUTTON_ID_DDOWN].UpdateSimpleButton( keyStates, XINPUT_GAMEPAD_DPAD_DOWN );
    m_buttons[XBOX_BUTTON_ID_DLEFT].UpdateSimpleButton( keyStates, XINPUT_GAMEPAD_DPAD_LEFT );
    m_buttons[XBOX_BUTTON_ID_DRIGHT].UpdateSimpleButton( keyStates, XINPUT_GAMEPAD_DPAD_RIGHT );
    m_buttons[XBOX_BUTTON_ID_SELECT].UpdateSimpleButton( keyStates, XINPUT_GAMEPAD_BACK );
    m_buttons[XBOX_BUTTON_ID_START].UpdateSimpleButton( keyStates, XINPUT_GAMEPAD_START );
    m_buttons[XBOX_BUTTON_ID_LBUMPER].UpdateSimpleButton( keyStates, XINPUT_GAMEPAD_LEFT_SHOULDER );
    m_buttons[XBOX_BUTTON_ID_RBUMPER].UpdateSimpleButton( keyStates, XINPUT_GAMEPAD_RIGHT_SHOULDER );
    m_buttons[XBOX_BUTTON_ID_LSTICK].UpdateSimpleButton( keyStates, XINPUT_GAMEPAD_LEFT_THUMB );
    m_buttons[XBOX_BUTTON_ID_RSTICK].UpdateSimpleButton( keyStates, XINPUT_GAMEPAD_RIGHT_THUMB );

    // Update Triggers
    m_leftTrigger = RangeMap( xboxControllerState.Gamepad.bLeftTrigger, 0, 255, 0, 1 );
    m_rightTrigger = RangeMap( xboxControllerState.Gamepad.bRightTrigger, 0, 255, 0, 1 );
    m_buttons[XBOX_BUTTON_ID_LTRIGGER].UpdateAnalogButton( m_leftTrigger );
    m_buttons[XBOX_BUTTON_ID_RTRIGGER].UpdateAnalogButton( m_rightTrigger );

    // Update Joysticks
    m_leftJoystick.UpdateInput( Vec2( xboxControllerState.Gamepad.sThumbLX, xboxControllerState.Gamepad.sThumbLY ) );
    m_rightJoystick.UpdateInput( Vec2( xboxControllerState.Gamepad.sThumbRX, xboxControllerState.Gamepad.sThumbRY ) );
}


void XboxController::Reset() {
    m_leftJoystick.Reset();
    m_rightJoystick.Reset();

    for( int i = 0; i < NUM_XBOX_BUTTON_IDS; i++ ) {
        m_buttons[i].Reset();
    }
}


bool XboxController::IsConnected() const {
    return m_isConnected;
}


int XboxController::GetControllerID() const {
    return m_controllerID;
}


const AnalogJoystick& XboxController::GetLeftJoystick() const {
    return m_leftJoystick;
}


const AnalogJoystick& XboxController::GetRightJoystick() const {
    return m_rightJoystick;
}


float XboxController::GetLeftTrigger() const {
    return m_leftTrigger;
}


float XboxController::GetRightTrigger() const {
    return m_rightTrigger;
}


const KeyButtonState& XboxController::GetKeyButtonState( XboxButtonID keyCode )  const {
    return m_buttons[keyCode];
}
