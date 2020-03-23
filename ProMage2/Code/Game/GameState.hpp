#pragma once
#include "Game/GameCommon.hpp"


enum LoadState {
    LOAD_STATE_PRE_INIT,
    LOAD_STATE_INIT,
    LOAD_STATE_READY,
    LOAD_STATE_DONE
};


class Camera;
class Clock;
class GameInput;
class GPUMesh;

enum MouseEvent : int;


class GameState {
    public:
    GameState();
    virtual ~GameState();

    virtual void Startup() = 0;
    virtual void Shutdown() = 0;

    virtual void Update() = 0;
    virtual void Render() = 0;

    virtual bool HandleKeyPressed( unsigned char keyCode ) = 0;
    virtual bool HandleKeyReleased( unsigned char keyCode ) = 0;
    virtual bool HandleMouseButton( MouseEvent event, float scrollAmount = 0.f ) = 0;
    virtual bool HandleQuitRequested() = 0;

    Camera* GetUICamera() const;
    Camera* GetGameCamera() const;
    virtual GameInput* GetGameInput() const;
    virtual Clock* GetStateClock() const;

    protected:
    LoadState m_loadState = LOAD_STATE_PRE_INIT;
    GPUMesh* m_loadingMesh = nullptr;
    GPUMesh* m_loadedMesh = nullptr;

    Clock* m_stateClock = nullptr;

    Camera* m_uiCamera = nullptr;       // Constructed by default, but no Ortho/Perspective set
    Camera* m_gameCamera = nullptr;     // NOT constructed by default, but still deleted

    virtual void BuildLoadingMesh();
    virtual void BuildLoadedMesh();
};
