#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Utils/NamedStrings.hpp"
#include "Engine/Utils/XMLUtils.hpp"

#include "Game/App.hpp"


NamedStrings g_theGameConfigBlackboard;
IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


// SD2FIXME: I feel like we should be able to replace this somehow...
bool GameWinProc( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam ) {
    UNUSED( windowHandle );
    UNUSED( lParam );
    unsigned char asKey = (unsigned char)wParam;

    if( !g_theDevConsole->IsTakingInput() && g_theGui->IsSetup() ) {
        bool handled = ImGui_ImplWin32_WndProcHandler( windowHandle, wmMessageCode, wParam, lParam );
        UNUSED( handled ); // FIXME: What does this variable mean???
        const ImGuiIO& io = ImGui::GetIO();

        switch( wmMessageCode ) {
            case( WM_KEYDOWN ): {
            } case( WM_KEYUP ): {
            } case( WM_CHAR ): {
                if( io.WantCaptureKeyboard ) {
                    return false; // ImGui WILL handle it later.. maybe during NewFrame???
                }

                break;
            } case(WM_LBUTTONDOWN): {
            } case(WM_LBUTTONUP): {
            } case(WM_RBUTTONDOWN): {
            } case(WM_RBUTTONUP): {
            } case(WM_MBUTTONDOWN): {
            } case(WM_MBUTTONUP): {
            } case(WM_MOUSEWHEEL): {
                if( io.WantCaptureMouse ) {
                    return false; // ImGui WILL handle it later.. maybe during NewFrame???
                }

                break;
            }
        }
    }

    switch( wmMessageCode ) {
        case WM_CLOSE: { // App close requested via "X" button, right-click "Close Window" on task bar, "Close" from system menu, Alt-F4, etc...
            return g_theApp->HandleQuitRequested();
        } case WM_KEYDOWN: { // Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
            return g_theApp->HandleKeyPressed( asKey );
        } case WM_KEYUP: { // Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
            return g_theApp->HandleKeyReleased( asKey );
        } case WM_CHAR: { // Also keydown but provided the direct key that was pressed in ASCII
            return g_theApp->HandleCharTyped( asKey );
        } case(WM_LBUTTONDOWN): {
            return g_theApp->HandleMouseButton( MOUSE_EVENT_LBUTTON_DOWN );
        } case(WM_LBUTTONUP): {
            return g_theApp->HandleMouseButton( MOUSE_EVENT_LBUTTON_UP );
        } case(WM_RBUTTONDOWN): {
            return g_theApp->HandleMouseButton( MOUSE_EVENT_RBUTTON_DOWN );
        } case(WM_RBUTTONUP): {
            return g_theApp->HandleMouseButton( MOUSE_EVENT_RBUTTON_UP );
        } case(WM_MBUTTONDOWN): {
            return g_theApp->HandleMouseButton( MOUSE_EVENT_MBUTTON_DOWN );
        } case(WM_MBUTTONUP): {
            return g_theApp->HandleMouseButton( MOUSE_EVENT_MBUTTON_UP );
        } case(WM_MOUSEWHEEL): {
            return g_theApp->HandleMouseButton( MOUSE_EVENT_SCROLL, GET_WHEEL_DELTA_WPARAM( wParam ) / (float)WHEEL_DELTA );
        }
    }

    return false;
}


//-----------------------------------------------------------------------------------------------
void Startup() {
    // Game config blackboard
    tinyxml2::XMLDocument projectConfig = new tinyxml2::XMLDocument();
    tinyxml2::XMLError loadSuccess = projectConfig.LoadFile( DATA_PROJECT_CONFIG );
    GUARANTEE_OR_DIE( loadSuccess == tinyxml2::XML_SUCCESS, Stringf( "Failed to load XML file: %s", DATA_PROJECT_CONFIG ) );

    tinyxml2::XMLNode* root = projectConfig.FirstChild();
    GUARANTEE_OR_DIE( root != nullptr, "Poorly constructed XML file" );
    g_theGameConfigBlackboard.SetFromXMLElementAttributes( *root->ToElement() );

#ifdef _DEBUG
    g_theGameConfigBlackboard.SetValue( "debugFlags", "NO_PRELOAD_TEXTURES" );
#endif


    // Make the App
    g_theApp = new App( GameWinProc );
    g_theApp->Startup();
}


//-----------------------------------------------------------------------------------------------
void Shutdown() {
    // Destroy the global App instance
    g_theApp->Shutdown();
    delete g_theApp;
    g_theApp = nullptr;
}


//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int ) {
    UNUSED( applicationInstanceHandle );
    UNUSED( commandLineString );
    Startup();

    // Program main loop; keep running frames until it's time to quit
    while( !g_theApp->IsQuitting() ) {
        g_theApp->RunFrame();
        Sleep( 0 );
    }

    Shutdown();
    return 0;
}
