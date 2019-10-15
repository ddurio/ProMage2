#pragma once
#include "Editor/EditorCommon.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/WindowContext.hpp"


class Camera;
class GPUMesh;
class Map;
class MapWindow;
class StepWindow;
class XmlWindow;


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

    MapWindow* GetMapWindow() const;
    XmlWindow* GetXmlWindow() const;


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

    std::vector< int > m_customResultIndex;

    Camera* m_uiCamera = nullptr;
    Clock m_editorClock;

    MapWindow* m_mapWindow      = nullptr;
    StepWindow* m_stepWindow    = nullptr;
    XmlWindow* m_xmlWindow      = nullptr;

    bool m_demoIsShown = false;


    bool UpdateIsLoaded();
    bool RenderIsLoaded() const;

    void BuildLoadingMesh();
    void BuildLoadedMesh();

    bool ToggleDemo( EventArgs& args );
};
