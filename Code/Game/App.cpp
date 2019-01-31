#include "Game/App.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/Game.hpp"


App* g_theApp;
Game* g_theGame;

AudioSystem* g_theAudio;
InputSystem* g_theInput;
RenderContext* g_theRenderer;
RNG* g_RNG;

extern void* g_hWnd;

App::App() {

}


App::~App() {

}


void App::Startup() {
    g_theDevConsole->Startup();

    g_theRenderer   = new RenderContext();
    g_theInput      = new InputSystem();
    g_theAudio      = new AudioSystem();
    g_RNG           = new RNG();

    g_theRenderer->Startup( g_hWnd );
    g_theInput->Startup();
    g_theAudio->Startup();

    g_theGame = new Game();
    g_theGame->Startup();
}


void App::Shutdown() {
    g_theGame->Shutdown();
    delete g_theGame;
    g_theGame = nullptr;

    g_theAudio->Shutdown();
    g_theInput->Shutdown();
    g_theRenderer->Shutdown();

    g_theDevConsole->Shutdown();

    delete g_RNG;
    g_RNG = nullptr;

    delete g_theAudio;
    g_theAudio = nullptr;

    delete g_theInput;
    g_theInput = nullptr;

    delete g_theRenderer;
    g_theRenderer = nullptr;

    delete g_theDevConsole;
    g_theDevConsole = nullptr;

}


void App::RunFrame() {
    BeginFrame();
    Update();
    Render();
    EndFrame();
}


void App::BeginFrame() {
    g_theInput->BeginFrame();
    g_theRenderer->BeginFrame();
    g_theAudio->BeginFrame();
    g_theDevConsole->BeginFrame();
}


void App::EndFrame() {
    g_theInput->EndFrame();
    g_theRenderer->EndFrame();
    g_theAudio->EndFrame();
    g_theDevConsole->EndFrame();
}


void App::Update() {
    double currentTime = GetCurrentTimeSeconds();
    float deltaSeconds = (float)(currentTime - m_timeLastFrame);
    m_timeLastFrame = currentTime;
    deltaSeconds = ClampFloat( deltaSeconds, 0.f, APP_MAX_DELTA_SECONDS );

    if( m_isSlowMo ) {
        deltaSeconds /= 10.f;
    } else if( m_isFastMo ) {
        deltaSeconds *= 4.f;
    }

    g_theGame->Update( deltaSeconds );
}


void App::Render() const {
    g_theRenderer->ClearColorTarget( Rgba( 0.f, 0.f, 0.f, 1.f ) );

    g_theGame->Render();

    g_theDevConsole->Render( g_theRenderer, g_theGame->GetActiveCamera(), 2.f );
}


bool App::HandleKeyPressed( unsigned char keyCode ) {
    if( g_theDevConsole->IsTakingInput() ) {
        if( !g_theDevConsole->HandleKeyPressed( keyCode ) ) {
            return true;
        }
    }

    //g_theDevConsole->PrintString( Stringf( "KeyCode pressed: %x", keyCode ) );

    switch( keyCode ) {
        case(0x1B): {  // Escape Key
            HandleQuitRequested();
            break;
        } case(0x77): { // F8 Key
            Shutdown();
            Startup();
            return false;
        } case('T'): {
            m_isSlowMo = true;
            return false;
        } case('Y'): {
            m_isFastMo = true;
            return false;
        } case(0xC0): { // ~ Key
            g_theDevConsole->Toggle();
            return false;
        }
    }

    return g_theGame->HandleKeyPressed( keyCode );
}


bool App::HandleKeyReleased( unsigned char keyCode ) {
    switch( keyCode ) {
        case('T'):
        {
            m_isSlowMo = false;
            return 0;
        } case('Y'):
        {
            m_isFastMo = false;
            return 0;
        }
    }

    return g_theGame->HandleKeyReleased( keyCode );
}


bool App::HandleQuitRequested() {
    m_isQuitting = true;
    return 0;
}
