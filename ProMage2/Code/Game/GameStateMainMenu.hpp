#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/EventSystem.hpp"

#include "Game/GameState.hpp"


class UIWidget;


class GameStateMainMenu : public GameState {
    public:
    GameStateMainMenu();
    ~GameStateMainMenu() override;

    void Startup() override;
    void Shutdown() override;

    void Update() override;
    void Render() override;

    bool HandleKeyPressed( unsigned char keyCode ) override;
    bool HandleKeyReleased( unsigned char keyCode ) override;
    bool HandleMouseButton( MouseEvent event, float scrollAmount = 0.f ) override;
    bool HandleQuitRequested() override;

    static bool Command_ChangeGameState( EventArgs& args );

    private:
    UIWidget* m_uiParent = nullptr;
};
