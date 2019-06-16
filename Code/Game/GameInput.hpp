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

    Vec2 GetMovementDirection() const;
    bool WasInvtoryToggled() const;
    bool ShouldPickupItem() const;
    //bool ShouldChangeFloors() const;

    bool IsPaused() const;

    private:
    // State
    bool m_pause = false;
    bool m_toggleInventory = false;
    bool m_pickupItem = false;
    //bool m_changeFloors = false;

    bool m_upPressed = false;
    bool m_downPressed = false;
    bool m_leftPressed = false;
    bool m_rightPressed = false;
};
