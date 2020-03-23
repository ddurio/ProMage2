#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/IntVec2.hpp"


class InputSystem;
struct AABB2;


enum MouseMode {
    MOUSE_MODE_ABSOLUTE,
    MOUSE_MODE_RELATIVE
};


enum MouseEvent {
    MOUSE_EVENT_NONE,

    MOUSE_EVENT_LBUTTON_DOWN,
    MOUSE_EVENT_LBUTTON_UP,
    MOUSE_EVENT_RBUTTON_DOWN,
    MOUSE_EVENT_RBUTTON_UP,
    MOUSE_EVENT_MBUTTON_DOWN,
    MOUSE_EVENT_MBUTTON_UP,
    MOUSE_EVENT_SCROLL
};

typedef bool( *WindowsProcCallback )(void*, unsigned int msg, uintptr_t wparam, uintptr_t lparam);


class WindowContext {
    public:
    WindowContext( const char* title, float aspect, float screenPercent, WindowsProcCallback callback );
    ~WindowContext();

    void Startup();
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    // Window
    void RunMessagePump();
    void RegisterInputSystem( InputSystem* inputSystem );
    InputSystem* GetInputSystem() const;

    bool HasFocus() const;
    void* GetWindowHandle() const;
    IntVec2 GetClientCenter() const;
    IntVec2 GetClientDimensions() const;
    AABB2 GetClientBounds() const;
    AABB2 GetClientBoundsInverted() const;
    WindowsProcCallback GetGameProc() const;

    std::string OpenFileDialog( const std::string& relativePath = "", const Strings& filter = { "Any File", "*.*" }, const std::string& windowTitle = "Open File" ) const;
    std::string SaveFileDialog( const std::string& relativePath = "", const Strings& filter = { "Any File", "*.*" }, const std::string& windowTitle = "Save File" ) const;

    // Mouse
    IntVec2 GetMouseClientPosition() const;
    IntVec2 GetMouseClientDisplacement() const;
    bool IsMouseLocked() const;
    void ShowMouse() const;
    void HideMouse() const;

    void SetMouseMode( MouseMode mode );
    void SetMouseClientPosition( const IntVec2& clientPosition );
    void LockMouse( bool isNewLock = true );
    void UnlockMouse();

    private:
    struct WindowsFile {
        char* filter        = nullptr;
        char* fileName      = nullptr;
        char* initialDir    = nullptr;
        void* ofn           = nullptr;

        ~WindowsFile();
    };


    // Window
    void* m_windowHandle = nullptr;
    WindowsProcCallback m_gameWindowsProc = nullptr;
    InputSystem* m_inputSystem = nullptr;

    // Mouse
    MouseMode m_mouseMode = MOUSE_MODE_ABSOLUTE;
    IntVec2 m_mousePositionCurrent = IntVec2::ZERO;
    IntVec2 m_mousePositionPrevious = IntVec2::ZERO;
    int m_mouseLockCount = 0;

    static int s_numWindows;


    // Window
    void RegisterWindowsClass() const;
    void* GetNewWindow( const char* title, float clientAspect, float clientPercentage ) const;

    WindowsFile* CreateWindowsFile( const std::string& relativePath, const Strings& filter, const std::string& windowTitle ) const;

    // Mouse
    void LockMouseToWindow() const;
};
