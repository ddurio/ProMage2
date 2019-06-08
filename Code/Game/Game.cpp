#include "Game/Game.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/Time.hpp"

#include "Game/App.hpp"
#include "Game/GameStateMainMenu.hpp"
#include "Game/GameStatePlay.hpp"


Game::Game( bool showMainMenu /*= true */ ) {
    m_gameClock = new Clock();

    if( showMainMenu ) {
        m_gameState = new GameStateMainMenu();
    } else {
        m_gameState = new GameStatePlay();
    }
}


Game::~Game() {
    CLEAR_POINTER( m_gameState );
}


void Game::Startup() {

}


void Game::Shutdown() {
    m_gameState->Shutdown();
}


void Game::Update() {
    m_gameState->Update();

    ImGui::NewFrame();

    ImGui::Begin( "Hello, world!" );
    ImGui::Text( "This is useful text.." );
    ImGui::Checkbox( "Demo Window", &ui_testCheck1 );
    ImGui::Checkbox( "Another window", &ui_testCheck2 );

    ImGui::SliderFloat( "float", &ui_testSlider, 0.f, 1.f );
    ImGui::ColorEdit3( "clear color", (float*)&ui_testColor );

    ImGui::End();
}


void Game::Render() const {
    m_gameState->Render();
}


bool Game::HandleKeyPressed( unsigned char keyCode ) {
    return  m_gameState->HandleKeyPressed( keyCode );
}


bool Game::HandleKeyReleased( unsigned char keyCode ) {
    return m_gameState->HandleKeyReleased( keyCode );
}


bool Game::HandleMouseButton( MouseEvent event, float scrollAmount /*= 0.f */ ) {
    return m_gameState->HandleMouseButton( event, scrollAmount );
}


bool Game::HandleQuitRequested() {
    return false;
}


void Game::ChangeGameState( GameState* state ) {
    CLEAR_POINTER( m_gameState );
    m_gameState = state;
}


Clock* Game::GetGameClock() const {
    return m_gameClock;
}


GameState* Game::GetGameState() const {
    return m_gameState;
}


Camera* Game::GetUICamera() const {
    return m_gameState->GetUICamera();
}


GameInput* Game::GetGameInput() const {
    return m_gameState->GetGameInput();
}
