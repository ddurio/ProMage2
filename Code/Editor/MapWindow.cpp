#if defined(_EDITOR)
#include "Editor/MapWindow.hpp"

#include "Editor/EditorMapDef.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView2D.hpp"


MapWindow::MapWindow( const Vec2& normDimensions /*= Vec2( 0.8f, 0.9f )*/, const Vec2& alignment /*= Vec2( 0.f, 1.f ) */ ) :
    EditorWindow( normDimensions, alignment ) {
    // ThesisFIXME: Choose better name for this window
    m_windowName = "MapEditor";

    const EditorMapDef* eMapDef = EditorMapDef::GetDefinition( "Cavern" );
    eMapDef->DefineObject( &m_mapPerStep );

    m_stepIndex = (int)m_mapPerStep.size() - 1;
    g_theEventSystem->Subscribe( EVENT_EDITOR_STEP_INDEX, this, &MapWindow::SetMapStepIndex );

    // Setup camera
    m_mapCamera = new Camera();
    IntVec2 mapDims = m_mapPerStep[0]->GetMapDimensions(); // ThesisFIXME: camera dimensions will be wrong if map size ever changes
    m_mapCamera->SetOrthoProjection( (float)mapDims.y );

    Vec2 halfDims = mapDims * 0.5f;
    m_mapCamera->Translate( halfDims );

    // Setup render target
    TextureView2D* mapView = g_theRenderer->GetOrCreateRenderTarget( m_mapViewName );
    m_mapCamera->SetRenderTarget( mapView );
}


MapWindow::~MapWindow() {
    CLEAR_POINTER( m_mapCamera );
    EngineCommon::ClearVector( m_mapPerStep );
}


Strings MapWindow::GetStepNames() const {
    Strings stepNames;
    stepNames.push_back( "0: Fill And Edge" );

    // Gen Steps
    std::string defType = m_mapPerStep[0]->GetMapType();
    const EditorMapDef* mapDef = EditorMapDef::GetDefinition( defType );
    Strings genStepNames = mapDef->GetStepNames( 1 );

    stepNames.insert( stepNames.end(), genStepNames.begin(), genStepNames.end() );

    // Edged
    int stepIndex = (int)stepNames.size();
    std::string edgedName = Stringf( "%d: Edged Tiles", stepIndex );
    stepNames.push_back( edgedName );

    // Colliders
    std::string colliderName = Stringf( "%d: Phys. Colliders", stepIndex + 1 );
    stepNames.push_back( colliderName );

    return stepNames;
}


void MapWindow::UpdateChild( float deltaSeconds ) {
    Map*& theMap = m_mapPerStep[m_stepIndex];

    g_theRenderer->BeginCamera( m_mapCamera );
    theMap->Render();
    g_theRenderer->EndCamera( m_mapCamera );

    void* mapView = m_mapCamera->GetRenderTarget()->GetShaderView();
    ImVec2 windowSize = ImGui::GetWindowSize(); // ThesisFIXME: This is bigger than I expected it to be (auto added scroll bar)
    ImGui::Image( mapView, windowSize );
}


bool MapWindow::SetMapStepIndex( EventArgs& args ) {
    int newIndex = args.GetValue( "stepIndex", -1 );
    int numSteps = (int)m_mapPerStep.size();

    if( newIndex < 0 || newIndex >= numSteps ) {
        std::string warningMsg = Stringf( "(MapEditor) Attempting to set stepIndex (%d) with %d steps", newIndex, numSteps );
        g_theDevConsole->PrintString( warningMsg, DevConsole::CHANNEL_WARNING ); // TheisFIXME: Add this to the editor channel when it exists
        return false;
    }

    m_stepIndex = newIndex;
    return true; // ThesisFIXME: be careful other things don't also need this event
}

#endif
