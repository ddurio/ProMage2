#pragma once
#include "Editor/EditorCommon.hpp"

#include "Engine/Async/Job.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Vertex/VertexUtils.hpp"


class Camera;
class GPUMesh;
class Map;
class MapWindow;
class StepWindow;
class XmlWindow;


class Editor : public Job {
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

    void SetMapLoadingState( bool isLoading );

    MapWindow* GetMapWindow() const;
    XmlWindow* GetXmlWindow() const;

    const Clock* GetEditorClock() const;


    private:

    enum LoadState {
        LOAD_STATE_LOADING,
        LOAD_STATE_DONE
    };


    bool m_editorIsLoading = true;
    bool m_mapIsLoading = false;
    int m_loadingIndex = 0;
    VertexList m_loadingVerts;

    std::vector< int > m_customResultIndex;

    Camera* m_uiCamera = nullptr;
    Clock m_editorClock;

    MapWindow* m_mapWindow      = nullptr;
    StepWindow* m_stepWindow    = nullptr;
    XmlWindow* m_xmlWindow      = nullptr;

    bool m_demoIsShown = false;

    std::string m_newMapType    = "";
    std::string m_newMapFill    = "";
    std::string m_mapDefFile    = DATA_MAP_DEFS;


    bool IsLoading() const;

    void Execute() override;
    void UpdateLoading();
    void UpdateMenuBar();

    void RenderLoading() const;

    bool ToggleDemo( EventArgs& args );
};
