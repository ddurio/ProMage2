#pragma once
#include "Editor/EditorCommon.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/WindowContext.hpp"


class Camera;
class GPUMesh;
class Map;
class MapEditor;
class StepEditor;
class XmlEditor;


class Editor {
    public:
    Editor();
    ~Editor();

    void Startup();
    void Shutdown();

    bool HandleKeyPressed( unsigned char keyCode );
    bool HandleKeyReleased( unsigned char keyCode );
    bool HandleMouseButton( MouseEvent event, float scrollAmount = 0.f );

    void Update();
    void Render() const;

    Vec2 CreateWindow( const Vec2& normDimensions, const Vec2& alignment, const char* name = "", ImGuiWindowFlags extraFlags = ImGuiWindowFlags_None ) const;


    private:

    enum LoadState {
        LOAD_STATE_PRE_INIT,
        LOAD_STATE_INIT,
        LOAD_STATE_READY,
        LOAD_STATE_DONE
    };


    LoadState m_loadState  = LOAD_STATE_PRE_INIT;
    GPUMesh* m_loadingMesh = nullptr;
    GPUMesh* m_loadedMesh  = nullptr;

    int m_spawnActorIndex = -1;
    int m_spawnItemIndex  = -1;

    Camera* m_uiCamera = nullptr;
    Clock m_editorClock;

    MapEditor* m_mapEditor      = nullptr;
    StepEditor* m_stepEditor    = nullptr;
    XmlEditor* m_xmlEditor      = nullptr;


    bool UpdateIsLoaded();
    bool RenderIsLoaded() const;

    void BuildLoadingMesh();
    void BuildLoadedMesh();
};
