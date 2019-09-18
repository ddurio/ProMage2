#include "Game/App.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Debug/DebugDraw.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/Game.hpp"

#if defined(_EDITOR)
#include "Editor/Editor.hpp"

Editor*         g_theEditor;
#endif


App*            g_theApp;
Game*           g_theGame;

AudioSystem*    g_theAudio;
ImGuiSystem*    g_theGui;
InputSystem*    g_theInput;
PhysicsSystem*  g_thePhysicsSystem;
RenderContext*  g_theRenderer;
RNG*            g_RNG;
WindowContext*  g_theWindow;


App::App( void* appWinProc ) {
    g_theDevConsole->Startup();

    g_theWindow = new WindowContext( APP_TITLE, CLIENT_ASPECT, CLIENT_MAX_SCREEN_PERCENT, (WindowsProcCallback)appWinProc );
}


App::~App() {
    CLEAR_POINTER( g_theWindow );
}


void App::Startup() {
    g_theDevConsole->Startup( false ); // Enables F8 resets
    Clock::s_master.SetFrameLimit( APP_MAX_DELTA_SECONDS );

    g_theRenderer = new RenderContext();
    g_theInput = new InputSystem();
    g_theAudio = new AudioSystem();
    g_thePhysicsSystem = new PhysicsSystem( g_theRenderer );
    g_RNG = new RNG();
    g_theGui = new ImGuiSystem( g_theRenderer, g_theWindow );

    g_theWindow->Startup();
    g_theRenderer->Startup( g_theWindow );
    g_theDebugger->Startup( g_theRenderer );
    g_theInput->Startup();
    g_theAudio->Startup();
    g_thePhysicsSystem->Startup();
    g_theGui->Startup();

#if defined(_EDITOR)
    g_theEditor = new Editor();
#else
    g_theGame = new Game( false );
    g_theGame->Startup();
#endif


    g_theEventSystem->Subscribe( "quit", this, &App::Command_Quit );
}


void App::Shutdown() {
#if defined(_EDITOR)
    g_theEditor->Shutdown();
    CLEAR_POINTER( g_theEditor );
#else
    g_theGame->Shutdown();
    CLEAR_POINTER( g_theGame );
#endif

    g_theGui->Shutdown();
    g_thePhysicsSystem->Shutdown();
    g_theAudio->Shutdown();
    g_theInput->Shutdown();
    g_theDebugger->Shutdown();
    g_theDevConsole->Shutdown();
    g_theRenderer->Shutdown();
    g_theWindow->Shutdown();

    CLEAR_POINTER( g_theGui );
    CLEAR_POINTER( g_RNG );
    CLEAR_POINTER( g_thePhysicsSystem );
    CLEAR_POINTER( g_theAudio );
    CLEAR_POINTER( g_theInput );
    CLEAR_POINTER( g_theRenderer );
}


void App::RunFrame() {
    BeginFrame();
    Update();
    Render();
    EndFrame();
}


bool App::HandleKeyPressed( unsigned char keyCode ) {
    // Open / Close DevConsole
    if( keyCode == KB_TILDE ) {
        g_theDevConsole->Toggle();
        return true;
    }

    // Pass keystrokes to DevConsole if open
    if( g_theDevConsole->IsTakingInput() ) {
        return g_theDevConsole->HandleKeyPressed( keyCode );
    }

    switch( keyCode ) {
        case( KB_F8 ): { // Restart game
            Shutdown();
            Startup();
            return true;
        }
    }

#if defined(_EDITOR)
    return g_theEditor->HandleKeyPressed( keyCode );
#else
    return g_theGame->HandleKeyPressed( keyCode );
#endif
}


bool App::HandleKeyReleased( unsigned char keyCode ) {
    if( g_theDevConsole->IsTakingInput() ) {
        return g_theDevConsole->HandleKeyReleased( keyCode );
    }

#if defined(_EDITOR)
    return g_theEditor->HandleKeyReleased( keyCode );
#else
    return g_theGame->HandleKeyReleased( keyCode );
#endif
}


bool App::HandleCharTyped( unsigned char character ) {
    if( g_theDevConsole->IsTakingInput() ) {
        return g_theDevConsole->HandleCharTyped( character );
    }

    return false;
}


bool App::HandleMouseButton( MouseEvent event, float scrollAmount /*= 0.f*/ ) {
#if defined(_EDITOR)
    return g_theEditor->HandleMouseButton( event, scrollAmount );
#else
    return g_theGame->HandleMouseButton( event, scrollAmount );
#endif
}


bool App::HandleQuitRequested() {
    m_isQuitting = true;
    return 0;
}


bool App::Command_Quit( EventArgs& args ) {
    UNUSED( args );
    return HandleQuitRequested();
}


bool App::IsQuitting() const {
    return m_isQuitting;
}


void App::BeginFrame() {
    g_theWindow->BeginFrame();
    Clock::s_master.Tick(); // Update master clock
    g_theInput->BeginFrame();
    g_theRenderer->BeginFrame();
    g_theAudio->BeginFrame();
    g_thePhysicsSystem->BeginFrame();
    g_theGui->BeginFrame();
    g_theDevConsole->BeginFrame();
    g_theDebugger->BeginFrame();
}


void App::EndFrame() {
    g_theInput->EndFrame();
    g_theRenderer->EndFrame();
    g_theAudio->EndFrame();
    g_thePhysicsSystem->EndFrame();
    g_theGui->EndFrame();
    g_theDevConsole->EndFrame();
    g_theDebugger->EndFrame();
}


void App::Update() {
#if defined(_EDITOR)
    g_theEditor->Update();
#else
    g_theGame->Update();
#endif
}


void App::Render() const {
    g_theRenderer->ClearRenderTarget( Rgba( 0.f, 0.f, 0.f, 1.f ) );

#if defined(_EDITOR)
    g_theEditor->Render();
#else
    g_theGame->Render();
#endif

    g_theDebugger->RenderScreen();
    g_theGui->Render();
    g_theDevConsole->Render( g_theRenderer );
}


