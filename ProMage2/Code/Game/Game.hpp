#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Vertex/VertexPCU.hpp"

#include "Game/Entity.hpp"


class Camera;
class Clock;
class GameInput;
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
    Camera* GetGameCamera() const;

    GameInput* GetGameInput() const;

    private:
    Clock* m_gameClock = nullptr;
    GameState* m_gameState = nullptr;

    // TESTING
    bool ui_testCheck1 = false;
    bool ui_testCheck2 = false;
    float ui_testSlider = 0.f;
    Rgba ui_testColor = Rgba::WHITE;
};
