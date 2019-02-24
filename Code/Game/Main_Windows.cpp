#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Game/App.hpp"


NamedStrings g_theGameConfigBlackboard;


// SD2FIXME: I feel like we should be able to replace this somehow...
bool GameWinProc( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam ) {
    UNUSED( windowHandle );
    UNUSED( lParam );
    unsigned char asKey = (unsigned char)wParam;

    switch( wmMessageCode ) {
        case WM_CLOSE:
        { // App close requested via "X" button, right-click "Close Window" on task bar, "Close" from system menu, Alt-F4, etc...
            return g_theApp->HandleQuitRequested();
        } case WM_KEYDOWN:
        { // Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
            return g_theApp->HandleKeyPressed( asKey );
        } case WM_KEYUP:
        { // Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
            return g_theApp->HandleKeyReleased( asKey );
        } case WM_CHAR:
        { // Also keydown but provided the direct key that was pressed in ASCII
            return g_theApp->HandleCharTyped( asKey );
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
