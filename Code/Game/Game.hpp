#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/VertexPCU.hpp"

#include "Game/Entity.hpp"


class Camera;
class Clock;
class GameState;

enum MouseEvent : int;


class Game {
    public:
    Game( bool showMainMenu = true );
    ~Game();

    void Startup();
    void Shutdown();

    void Update();
    void Render() const;

    bool HandleKeyPressed( unsigned char keyCode );
    bool HandleKeyReleased( unsigned char keyCode );
    bool HandleMouseButton( MouseEvent event, float scrollAmount = 0.f );
    bool HandleQuitRequested();

    void ChangeGameState( GameState* state );

    Clock* GetGameClock() const;
    GameState* GetGameState() const;
    Camera* GetUICamera() const;

    private:
    Clock* m_gameClock = nullptr;
    GameState* m_gameState = nullptr;
};
