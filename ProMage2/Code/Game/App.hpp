#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/WindowContext.hpp"


class App {
    public:
    App( void* appWinProc );
    ~App();

    void Startup();
    void Shutdown();
    void RunFrame();

    bool HandleKeyPressed( unsigned char keyCode );
    bool HandleKeyReleased( unsigned char keyCode );
    bool HandleCharTyped( unsigned char character );
    bool HandleMouseButton( MouseEvent event, float scrollAmount = 0.f );

    bool HandleQuitRequested();
    bool Command_Quit( EventArgs& args );
    bool IsQuitting() const;

    private:
    bool m_isQuitting = false;


    void BeginFrame();
    void Update();
    void Render() const;
    void EndFrame();
};
