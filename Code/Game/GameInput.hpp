#pragma once
#include "Game/GameCommon.hpp"


enum MouseEvent : int;


class GameInput {
    public:
    GameInput();
    ~GameInput();

    void Startup();
    void Shutdown();

    void Update( float deltaSeconds );

    bool HandleKeyPressed( unsigned char keyCode );
    bool HandleKeyReleased( unsigned char keyCode );
    bool HandleMouseButton( MouseEvent event, float scrollAmount = 0.f );

    Vec2  GetFrameScroll() const;
    float GetFrameZoom();
    float GetFrameRotation() const;

    bool IsPaused() const;

    private:
    // State
    bool m_pause = false;
    bool m_upPressed = false;
    bool m_downPressed = false;
    bool m_leftPressed = false;
    bool m_rightPressed = false;
    bool m_rotateLeftPressed = false;
    bool m_rotateRightPressed = false;

    bool m_mouseLeft = false;
    bool m_mouseRight = false;
    bool m_mouseUp = false;
    bool m_mouseDown = false;

    float m_zoomAmount = 0.f;

    // Configuration
    float m_keyBoardScrollSpeed = 16.f;

    float m_mouseScrollSpeed = 16.f;
    float m_mouseScrollDistance = 32.f; // Distance from edge to start scrolling

    float m_rotationSpeed = 45.f;
    float m_zoomSpeed = 10.f;
};
