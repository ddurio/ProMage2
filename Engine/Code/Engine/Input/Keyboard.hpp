#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Input/KeyButtonState.hpp"


enum MouseEvent : int;

enum MouseButton {
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE,

    NUM_MOUSE_BUTTONS
};


class KeyboardAndMouse {
    public:
    KeyboardAndMouse() {};
    ~KeyboardAndMouse() {};

    void BeginFrame();
    void EndFrame();

    bool HandleKeyPressed( unsigned char keyCode );
    bool HandleKeyReleased( unsigned char keyCode );
    bool HandleMouseButton( MouseEvent event, float scrollAmount = 0.f );

    const KeyButtonState& GetKeyButtonState( unsigned char keyCode ) const;
    const KeyButtonState& GetMouseButtonState( MouseButton buttonCode ) const;


    private:
    static const int NUM_KEYBOARD_KEYS = 256;

    KeyButtonState m_keyboardStates[NUM_KEYBOARD_KEYS];
    KeyButtonState m_mouseStates[NUM_MOUSE_BUTTONS];
    float m_mouseScroll = 0.f;
};


#define KB_BACKSPACE        0x08
#define KB_ENTER            0x0D
#define KB_RETURN           0x0D
#define KB_SHIFT            0x10
#define KB_CONTROL          0x11
#define KB_ESCAPE           0x1B
#define KB_SPACE            0x20
#define KB_PAGE_UP          0x21
#define KB_PAGE_DOWN        0x22
#define KB_END              0x23
#define KB_HOME             0x24
#define KB_L_ARROW          0x25
#define KB_U_ARROW          0x26
#define KB_R_ARROW          0x27
#define KB_D_ARROW          0x28
#define KB_DELETE           0x2E
#define KB_0                0x30
#define KB_1                0x31
#define KB_2                0x32
#define KB_3                0x33
#define KB_4                0x34
#define KB_5                0x35
#define KB_6                0x36
#define KB_7                0x37
#define KB_8                0x38
#define KB_9                0x39
#define KB_A                0x41
#define KB_B                0x42
#define KB_C                0x43
#define KB_D                0x44
#define KB_E                0x45
#define KB_F                0x46
#define KB_G                0x47
#define KB_H                0x48
#define KB_I                0x49
#define KB_J                0x4A
#define KB_K                0x4B
#define KB_L                0x4C
#define KB_M                0x4D
#define KB_N                0x4E
#define KB_O                0x4F
#define KB_P                0x50
#define KB_Q                0x51
#define KB_R                0x52
#define KB_S                0x53
#define KB_T                0x54
#define KB_U                0x55
#define KB_V                0x56
#define KB_W                0x57
#define KB_X                0x58
#define KB_Y                0x59
#define KB_Z                0x5A
#define KB_NUM_0            0x60
#define KB_NUM_1            0x61
#define KB_NUM_2            0x62
#define KB_NUM_3            0x64
#define KB_NUM_4            0x64
#define KB_NUM_5            0x65
#define KB_NUM_6            0x66
#define KB_NUM_7            0x67
#define KB_NUM_8            0x68
#define KB_NUM_9            0x69
#define KB_F1               0x70
#define KB_F2               0x71
#define KB_F3               0x72
#define KB_F4               0x73
#define KB_F5               0x74
#define KB_F6               0x75
#define KB_F7               0x76
#define KB_F8               0x77
#define KB_F9               0x78
#define KB_F10              0x79
#define KB_F11              0x7A
#define KB_F12              0x7B
#define KB_COLON            0xBA
#define KB_PLUS             0xBB
#define KB_EQUALS           0xBB
#define KB_COMMA            0xBC
#define KB_MINUS            0xBD
#define KB_PERIOD           0xBE
#define KB_SLASH            0xBF
#define KB_TILDE            0xC0
#define KB_OPEN_BRACKET     0xDB
#define KB_BACK_SLASH       0xDC
#define KB_CLOSE_BRACKET    0xDD
#define KB_QUOTE            0xDE
