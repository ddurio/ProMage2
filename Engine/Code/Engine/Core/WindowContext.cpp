#include "Engine/Core/WindowContext.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "commdlg.h"

#include "Engine/Debug/Profiler.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/AABB2.hpp"

#include "filesystem"
#include "regex"


static constexpr const TCHAR* GAME_WINDOW_CLASS_NAME = TEXT( "GameWindowClass" );

int WindowContext::s_numWindows = 0; // Initialize to zero, updated upon creation


//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called by Windows whenever we ask it for notifications
LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam ) {
    WindowContext* windowContext = (WindowContext*)GetWindowLongPtr( windowHandle, GWLP_USERDATA );

    // Engine level message handling (like WM_ACTIVATE)
    switch(wmMessageCode) {
        case(WM_ACTIVATE): {
            WORD activeWord = LOWORD( wParam );
            bool isActive = (activeWord != WA_INACTIVE);

            if( isActive && windowContext->IsMouseLocked() ) {
                windowContext->LockMouse( false );
            }

            break;
        }
    }

    // Pass info to input system too -- for the time being, app will still get the message too
    InputSystem* inputSystem = (windowContext != nullptr) ? windowContext->GetInputSystem() : nullptr;
    bool engineHandled = false;

    if( inputSystem != nullptr ) {
        unsigned char keyCode = (unsigned char)wParam;

        switch( wmMessageCode ) {
            case WM_KEYDOWN: { // Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
                engineHandled = inputSystem->HandleKeyPressed( keyCode );
                break;
            } case WM_KEYUP: { // Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
                engineHandled = inputSystem->HandleKeyReleased( keyCode );
                break;
            } case(WM_LBUTTONDOWN): {
                engineHandled = inputSystem->HandleMouseButton( MOUSE_EVENT_LBUTTON_DOWN );
                break;
            } case(WM_LBUTTONUP): {
                engineHandled = inputSystem->HandleMouseButton( MOUSE_EVENT_LBUTTON_UP );
                break;
            } case(WM_RBUTTONDOWN): {
                engineHandled = inputSystem->HandleMouseButton( MOUSE_EVENT_RBUTTON_DOWN );
                break;
            } case(WM_RBUTTONUP): {
                engineHandled = inputSystem->HandleMouseButton( MOUSE_EVENT_RBUTTON_UP );
                break;
            } case(WM_MBUTTONDOWN): {
                engineHandled = inputSystem->HandleMouseButton( MOUSE_EVENT_MBUTTON_DOWN );
                break;
            } case(WM_MBUTTONUP): {
                engineHandled = inputSystem->HandleMouseButton( MOUSE_EVENT_MBUTTON_UP );
                break;
            } case(WM_MOUSEWHEEL): {
                engineHandled = inputSystem->HandleMouseButton( MOUSE_EVENT_SCROLL, GET_WHEEL_DELTA_WPARAM( wParam ) / (float)WHEEL_DELTA );
                break;
            }
        }
    }

    // Game level message handling (callback associated with the windowContext)
    bool appHandled = false;

    if( windowContext != nullptr ) {
        WindowsProcCallback gameCallback = windowContext->GetGameProc();
        appHandled = gameCallback( windowHandle, wmMessageCode, wParam, lParam );
    }

    if( engineHandled || appHandled ) {
        return 0;
    } else {
        // Send back to Windows for any unhandled messages we want other apps to see (e.g. play/pause in music apps, etc.)
        return DefWindowProc( windowHandle, wmMessageCode, wParam, lParam );
    }
}


WindowContext::WindowContext( const char* title, float clientAspect, float clientPercentage, WindowsProcCallback gameWinProc ) :
    m_windowHandle(nullptr),
    m_gameWindowsProc(gameWinProc) {
    RegisterWindowsClass();

    // Create the window
    HWND windowHandle = (HWND)GetNewWindow( title, clientAspect, clientPercentage );
    GUARANTEE_OR_DIE( windowHandle != nullptr, "WindowContext: Failed to create new window" );

    m_windowHandle = windowHandle;
    SetWindowLongPtr( windowHandle, GWLP_USERDATA, (LONG_PTR)this ); // Needed in the WinMessagePump (GetWindowLongPtr)

    // Initial Setup
	ShowWindow( windowHandle, SW_SHOW );
	SetForegroundWindow( windowHandle );
	SetFocus( windowHandle );

	HCURSOR cursor = LoadCursor( NULL, IDC_ARROW );
	SetCursor( cursor );

    m_mousePositionPrevious = GetMouseClientPosition();
    m_mousePositionCurrent = GetMouseClientPosition();
}


WindowContext::~WindowContext() {
    // Close window
    if( m_windowHandle != nullptr ) {
        CloseWindow( (HWND)m_windowHandle );
        m_windowHandle = nullptr;
    }

    // Unregister windows class
    s_numWindows--;

    if( s_numWindows == 0 ) {
        UnregisterClass( GAME_WINDOW_CLASS_NAME, GetModuleHandle( NULL ) );
    }
}


void WindowContext::Startup() {

}


void WindowContext::Shutdown() {

}


void WindowContext::BeginFrame() {
    PROFILE_FUNCTION();

    RunMessagePump();

    m_mousePositionPrevious = m_mousePositionCurrent;
    m_mousePositionCurrent = GetMouseClientPosition();

    if( m_mouseMode == MOUSE_MODE_RELATIVE && HasFocus() ) {
        IntVec2 center = GetClientCenter();
        SetMouseClientPosition( center ); // Reset the mouse to the center
        m_mousePositionPrevious = GetMouseClientPosition(); // Previous position should always be the center but account for small variance
    }
}


void WindowContext::EndFrame() {

}


//-----------------------------------------------------------------------------------------------
// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
//	is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)
void WindowContext::RunMessagePump() {
	MSG queuedMessage;

    while( true ) {
		const BOOL wasMessagePresent = PeekMessage( &queuedMessage, NULL, 0, 0, PM_REMOVE );
		if( !wasMessagePresent ) {
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage ); // Tells Windows to call our WinProc (WindowsMessageHandlingProcedure)
	}
}


void WindowContext::RegisterInputSystem( InputSystem* inputSystem ) {
    m_inputSystem = inputSystem;
}

InputSystem* WindowContext::GetInputSystem() const {
    return m_inputSystem;
}


bool WindowContext::HasFocus() const {
    return (GetActiveWindow() == (HWND)m_windowHandle);
}


void* WindowContext::GetWindowHandle() const {
    return m_windowHandle;
}


IntVec2 WindowContext::GetClientCenter() const {
    RECT clientBounds;
    GetClientRect( (HWND)m_windowHandle, &clientBounds );

    IntVec2 center;
    center.x = (clientBounds.left + clientBounds.right) / 2;
    center.y = (clientBounds.top + clientBounds.bottom) / 2;

    return center;
}


IntVec2 WindowContext::GetClientDimensions() const {
    RECT clientRect;
    GetClientRect( (HWND)m_windowHandle, &clientRect );

    int width  = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    return IntVec2( width, height );
}


AABB2 WindowContext::GetClientBounds() const {
    RECT clientRect;
    GetClientRect( (HWND)m_windowHandle, &clientRect );

    Vec2 mins = Vec2( (float)clientRect.left, (float)clientRect.bottom );
    Vec2 maxs = Vec2( (float)clientRect.right, (float)clientRect.top );

    return AABB2( mins, maxs );
}


AABB2 WindowContext::GetClientBoundsInverted() const {
    RECT clientRect;
    GetClientRect( (HWND)m_windowHandle, &clientRect );

    Vec2 mins = Vec2( (float)clientRect.left, -(float)clientRect.bottom );
    Vec2 maxs = Vec2( (float)clientRect.right, (float)clientRect.top );

    return AABB2( mins, maxs );
}


WindowsProcCallback WindowContext::GetGameProc() const {
    return m_gameWindowsProc;
}


std::string WindowContext::OpenFileDialog(
    const std::string& relativePath /* = "" */,
    const Strings& filter /* = { "Any File", "*.*" } */,
    const std::string& windowTitle /* = "Open File" */ ) const
{
    std::filesystem::path workingDir = std::filesystem::current_path();

    WindowsFile* wFile = CreateWindowsFile( relativePath, filter, windowTitle );
    std::string selectedFileName = "";

    // Open Dialog
    if( GetOpenFileNameA( (OPENFILENAMEA*)wFile->ofn ) ) {
        std::filesystem::path path = std::filesystem::relative( wFile->fileName, workingDir );
        selectedFileName = path.string();
    }

    // Windows changes currentPath after opening file... just set it back to correct value
    std::filesystem::current_path( workingDir );
    delete wFile;

    return selectedFileName;
}


std::string WindowContext::SaveFileDialog(
    const std::string& relativePath /* = "" */,
    const Strings& filter /* = { "Any File", "*.*" } */,
    const std::string& windowTitle /* = "Save File" */ ) const
{
    std::filesystem::path workingDir = std::filesystem::current_path();

    WindowsFile* wFile = CreateWindowsFile( relativePath, filter, windowTitle );
    std::string selectedFilePath = "";

    // Save Dialog
    if( GetSaveFileNameA( (OPENFILENAMEA*)wFile->ofn ) ) {
        std::filesystem::path path = std::filesystem::relative( wFile->fileName, workingDir );
        selectedFilePath = path.string();
    }

    // Windows changes currentPath after opening file... just set it back to correct value
    std::filesystem::current_path( workingDir );
    delete wFile;

    return selectedFilePath;
}


IntVec2 WindowContext::GetMouseClientPosition() const {
    POINT desktopPosition;
    bool cursorFound = GetCursorPos( &desktopPosition );

    if( !cursorFound ) {
        desktopPosition.x = 0;
        desktopPosition.y = 0;
    }

    ScreenToClient( (HWND)m_windowHandle, &desktopPosition );
    return IntVec2( desktopPosition.x, desktopPosition.y );
}


IntVec2 WindowContext::GetMouseClientDisplacement() const {
    return m_mousePositionCurrent - m_mousePositionPrevious;
}


bool WindowContext::IsMouseLocked() const {
    return (m_mouseLockCount > 0);
}


void WindowContext::ShowMouse() const {
    ShowCursor( true );
}


void WindowContext::HideMouse() const {
    ShowCursor( false );
}


void WindowContext::SetMouseMode( MouseMode mode ) {
    m_mouseMode = mode;

    if( m_mouseMode == MOUSE_MODE_RELATIVE ) {
        IntVec2 center = GetClientCenter();

        // Set everything to center
        SetMouseClientPosition( center );
        m_mousePositionCurrent = GetMouseClientPosition();
        m_mousePositionPrevious = GetMouseClientPosition();
    }
}


void WindowContext::SetMouseClientPosition( const IntVec2& clientPosition ) {
    POINT desktopPosition = { clientPosition.x, clientPosition.y };

    ClientToScreen( (HWND)m_windowHandle, &desktopPosition );
    SetCursorPos( desktopPosition.x, desktopPosition.y );
}


void WindowContext::LockMouse( bool isNewLock /*= true */ ) {
    if( isNewLock ) {
        m_mouseLockCount++;
    }

    if( m_mouseLockCount > 0 )  {
        LockMouseToWindow();
    }

    m_mousePositionPrevious = GetMouseClientPosition();
    m_mousePositionCurrent = GetMouseClientPosition();
}


void WindowContext::UnlockMouse() {
    m_mouseLockCount--;

    if( m_mouseLockCount <= 0 ) {
        ClipCursor( nullptr );
    }
}


void WindowContext::RegisterWindowsClass() const {
    if( s_numWindows > 0 ) {
        s_numWindows++;
        return; // Only needed once
    }

    s_numWindows++;

	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle( NULL );
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = GAME_WINDOW_CLASS_NAME;
	RegisterClassEx( &windowClassDescription );
}


void* WindowContext::GetNewWindow( const char* title, float clientAspect, float clientPercentage ) const {
	// #SD1FIXME: Add support for fullscreen mode (requires different window style flags than windowed mode)
	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER |  WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect( desktopWindowHandle, &desktopRect );
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	float desktopAspect = desktopWidth / desktopHeight;

	// Calculate maximum client size (as some % of desktop size)
	float clientWidth = desktopWidth * clientPercentage;
	float clientHeight = desktopHeight * clientPercentage;

	if( clientAspect > desktopAspect ) {
		// Client window has a wider aspect than desktop; shrink client height to match its width
		clientHeight = clientWidth / clientAspect;
	} else {
		// Client window has a taller aspect than desktop; shrink client width to match its height
		clientWidth = clientHeight * clientAspect;
	}

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - clientHeight);

	RECT clientRect;
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + (int)clientWidth;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + (int)clientHeight;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[1024];
	MultiByteToWideChar( GetACP(), 0, title, -1, windowTitle, sizeof( windowTitle ) / sizeof( windowTitle[0] ) );
	HWND windowHandle = CreateWindowEx(
		windowStyleExFlags,
		GAME_WINDOW_CLASS_NAME,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL
    );

    return windowHandle;
}


WindowContext::WindowsFile* WindowContext::CreateWindowsFile(
    const std::string& relativePath,
    const Strings& filterArr,
    const std::string& windowTitle ) const
{

    std::filesystem::path workingDir = std::filesystem::current_path();

    // Combine filter strings
    WindowsFile* wFile = new WindowsFile();
    wFile->fileName   = new char[MAX_PATH];
    wFile->filter     = new char[MAX_PATH];
    wFile->initialDir = new char[MAX_PATH];
    wFile->ofn        = new OPENFILENAMEA();

    char* buffer = wFile->filter;
    int charRemaining = MAX_PATH;
    int numStrings = (int)filterArr.size();

    for( int strIndex = 0; strIndex < numStrings; strIndex++ ) {
        strcpy_s( buffer, charRemaining, filterArr[strIndex].c_str() );
        int strSize = (int)filterArr[strIndex].size();

        buffer += strSize;
        charRemaining -= strSize;

        buffer[0] = '\0';
        buffer += 1;
        charRemaining -= 1;
    }

    buffer[0] = '\0';

    // Setup initial directory
    std::string relativeDir = relativePath;
    size_t charIndex = relativeDir.find_first_of( '/' );

    while( charIndex != std::string::npos ) {
        relativeDir.replace( charIndex, 1, "\\" );
        charIndex = relativeDir.find_first_of( '/' );
    }

    std::string initialDirStr = Stringf( "%s\\%s", workingDir.string().c_str(), relativeDir.c_str() );
    memcpy( wFile->initialDir, initialDirStr.c_str(), initialDirStr.size() + 1 );

    // Create fileName object

    ZeroMemory( wFile->fileName, MAX_PATH );
    ZeroMemory( wFile->ofn, sizeof( OPENFILENAMEA ) );
    OPENFILENAMEA* ofn = (OPENFILENAMEA*)wFile->ofn;

    ofn->lStructSize       = sizeof( OPENFILENAMEA );
    ofn->hwndOwner         = (HWND)m_windowHandle;
    ofn->lpstrFilter       = wFile->filter;
    ofn->lpstrFile         = wFile->fileName;
    ofn->lpstrInitialDir   = wFile->initialDir;
    ofn->nMaxFile          = MAX_PATH;
    ofn->lpstrTitle        = windowTitle.c_str();
    ofn->Flags             = OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    return wFile;
}


void WindowContext::LockMouseToWindow() const {
    RECT clientBounds;
    GetClientRect( (HWND)m_windowHandle, &clientBounds );

    POINT clientOrigin = { 0, 0 };
    ClientToScreen( (HWND)m_windowHandle, &clientOrigin );

    clientBounds.left += clientOrigin.x;
    clientBounds.right += clientOrigin.x;
    clientBounds.top += clientOrigin.y;
    clientBounds.bottom += clientOrigin.y;

    ClipCursor( &clientBounds );
}


// File Struct
WindowContext::WindowsFile::~WindowsFile() {
    delete filter;
    delete fileName;
    delete initialDir;
    delete ofn;
}
