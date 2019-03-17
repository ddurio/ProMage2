#include "Game/App.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/DebugDraw.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/WindowContext.hpp"
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
WindowContext* g_theWindow;

extern void* g_hWnd;

App::App( void* appWinProc ) {
    g_theDevConsole->Startup();

    g_theWindow = new WindowContext( APP_TITLE, CLIENT_ASPECT, CLIENT_MAX_SCREEN_PERCENT, (WindowsProcCallback)appWinProc );
}


App::~App() {
    delete g_theWindow;
    g_theWindow = nullptr;

    g_theDevConsole->Shutdown();
}


void App::Startup() {
    g_theRenderer = new RenderContext();
    g_theInput = new InputSystem();
    g_theAudio = new AudioSystem();
    g_RNG = new RNG();

    g_theWindow->Startup();
    g_theRenderer->Startup( g_theWindow );
    g_theDebugger->Startup( g_theRenderer );
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
    g_theDebugger->Shutdown();
    g_theRenderer->Shutdown();
    g_theWindow->Shutdown();

    delete g_RNG;
    g_RNG = nullptr;

    delete g_theAudio;
    g_theAudio = nullptr;

    delete g_theInput;
    g_theInput = nullptr;

    delete g_theRenderer;
    g_theRenderer = nullptr;
}


void App::RunFrame() {
    BeginFrame();
    Update();
    Render();
    EndFrame();
}


bool App::HandleKeyPressed( unsigned char keyCode ) {
    if( g_theDevConsole->IsTakingInput() ) {
        if( keyCode == 0xC0 ) { // ~ Key
            g_theDevConsole->Toggle();
            return true;
        }

        return g_theDevConsole->HandleKeyPressed( keyCode );
    }

    //g_theDevConsole->PrintString( Stringf( "KeyCode pressed: %x", keyCode ) );

    switch( keyCode ) {
        case(0x1B): {  // Escape Key
            HandleQuitRequested();
            break;
        } case(0x77): { // F8 Key
            Shutdown();
            Startup();
            return true;
        } case('T'): {
            m_isSlowMo = true;
            return true;
        } case('Y'): {
            m_isFastMo = true;
            return true;
        } case(0xC0): { // ~ Key
            g_theDevConsole->Toggle();
            return true;
        }
    }

    return g_theGame->HandleKeyPressed( keyCode );
}


bool App::HandleKeyReleased( unsigned char keyCode ) {
    if( g_theDevConsole->IsTakingInput() ) {
        bool consoleHandled = g_theDevConsole->HandleKeyReleased( keyCode );

        if( consoleHandled ) { // If console didn't handle it, keep passing it on
            return true;
        }
    }

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


bool App::HandleCharTyped( unsigned char character ) {
    if( g_theDevConsole->IsTakingInput() ) {
        return g_theDevConsole->HandleCharTyped( character );
    }

    return false;
}


bool App::HandleQuitRequested() {
    m_isQuitting = true;
    return 0;
}


bool App::IsQuitting() const {
    return m_isQuitting;
}


void App::BeginFrame() {
    g_theWindow->BeginFrame();
    g_theInput->BeginFrame();
    g_theRenderer->BeginFrame();
    g_theAudio->BeginFrame();
    g_theDevConsole->BeginFrame();
    g_theDebugger->BeginFrame();
}


void App::EndFrame() {
    g_theInput->EndFrame();
    g_theRenderer->EndFrame();
    g_theAudio->EndFrame();
    g_theDevConsole->EndFrame();
    g_theDebugger->EndFrame();
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
    g_theDebugger->Update( deltaSeconds );
}


void App::Render() const {
    g_theRenderer->ClearColorTarget( Rgba( 0.f, 0.f, 0.f, 1.f ) );

    g_theGame->Render();

    g_theDebugger->RenderScreen();
    g_theDevConsole->Render( g_theRenderer );
}


