#if defined(_EDITOR)
#include "Editor/MapEditor.hpp"

#include "Editor/EditorMapDef.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView2D.hpp"


MapEditor::MapEditor( const Vec2& normDimensions /*= Vec2( 0.8f, 0.9f )*/, const Vec2& alignment /*= Vec2( 0.f, 1.f ) */ ) :
    EditorWindow( normDimensions, alignment ) {
    // ThesisFIXME: Choose better name for this window
    m_windowName = "MapEditor";

    const EditorMapDef* eMapDef = EditorMapDef::GetDefinition( "Cavern" );
    eMapDef->DefineObject( &m_mapPerStep );

    m_mapCamera = new Camera();

    // Setup camera
    IntVec2 mapDims = m_mapPerStep[0]->GetMapDimensions(); // ThesisFIXME: camera dimensions will be wrong if map size ever changes
    m_mapCamera->SetOrthoProjection( (float)mapDims.y );

    Vec2 halfDims = mapDims * 0.5f;
    m_mapCamera->Translate( halfDims );

    // Setup render target
    TextureView2D* mapView = g_theRenderer->GetOrCreateRenderTarget( m_mapViewName );
    m_mapCamera->SetRenderTarget( mapView );
}


MapEditor::~MapEditor() {
    CLEAR_POINTER( m_mapCamera );
    EngineCommon::ClearVector( m_mapPerStep );
}


void MapEditor::UpdateChild( float deltaSeconds ) {
    // ThesisFIXME: getting rotating step index.. needs to be based on member variable
    int numSteps = (int)m_mapPerStep.size();
    int stepIndex = GetIndexOverTime( numSteps, 1.f );
    Map*& theMap = m_mapPerStep[stepIndex];

    g_theRenderer->BeginCamera( m_mapCamera );
    theMap->Render();
    g_theRenderer->EndCamera( m_mapCamera );

    void* mapView = m_mapCamera->GetRenderTarget()->GetShaderView();
    ImVec2 windowSize = ImGui::GetWindowSize(); // ThesisFIXME: This is bigger than I expected it to be (auto added scroll bar)
    ImGui::Image( mapView, windowSize );
}

#endif