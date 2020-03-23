#pragma once
#include "Engine/Core/EngineCommon.hpp"


class KeyButtonState {
    public:
    KeyButtonState();
    ~KeyButtonState();
    bool IsPressed() const;
    bool WasJustPressed() const;
    bool WasJustReleased() const;

    void SetState( bool isNowPressed );
    void UpdateSimpleButton( unsigned short buttonStates, unsigned short bitmaskFlag );
    void UpdateAnalogButton( float pressedValue );
    void Reset();

    private:
    bool m_wasPressedLastFrame = false;
    bool m_isPressed = false;
};
