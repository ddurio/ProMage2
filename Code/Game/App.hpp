#pragma once
#include "Game/GameCommon.hpp"


class App {
    public:
    App();
    ~App();
    void Startup();
    void Shutdown();
    void RunFrame();

    bool IsQuitting() const {
        return m_isQuitting;
    }
    bool HandleKeyPressed( unsigned char keyCode );
    bool HandleKeyReleased( unsigned char keyCode );
    bool HandleQuitRequested();

    private:
    void BeginFrame();
    void Update();
    void Render() const;
    void EndFrame();

    private:
    double m_timeLastFrame = 0.0;

    bool m_isQuitting = false;
    bool m_isSlowMo = false;
    bool m_isFastMo = false;
};
