#if defined(_EDITOR)
#include "Editor/MapWindow.hpp"

#include "Editor/EditorMapDef.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Debug/DebugDraw.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Texture2D.hpp"
#include "Engine/Renderer/TextureView2D.hpp"


MapWindow::MapWindow( const Vec2& normDimensions /*= Vec2( 0.8f, 0.9f )*/, const Vec2& alignment /*= Vec2( 0.f, 1.f ) */ ) :
    EditorWindow( normDimensions, alignment ) {
    m_windowName = "MapEditor";

    const EditorMapDef* eMapDef = EditorMapDef::GetDefinition( "Island" );
    eMapDef->DefineObject( &m_mapPerStep );

    m_stepIndex = (int)m_mapPerStep.size() - 1;
    g_theEventSystem->Subscribe( EVENT_EDITOR_STEP_INDEX, this, &MapWindow::SetMapStepIndex );

    // Setup camera
    IntVec2 mapDims = m_mapPerStep[0]->GetMapDimensions();
    float mapAspect = (float)mapDims.x / (float)mapDims.y;

    m_mapCamera = new Camera();
    m_mapCamera->SetOrthoProjection( (float)mapDims.y, -100.f, 100.f, mapAspect );

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
    stepNames.push_back( "1: Fill And Edge" );

    // Gen Steps
    std::string defType = m_mapPerStep[0]->GetMapType();
    const EditorMapDef* mapDef = EditorMapDef::GetDefinition( defType );
    Strings genStepNames = mapDef->GetStepNames( 2 ); // One step and make it not zero indexed

    stepNames.insert( stepNames.end(), genStepNames.begin(), genStepNames.end() );

    // Edged
    int stepIndex = (int)stepNames.size();
    std::string edgedName = Stringf( "%d: Edged Tiles", stepIndex + 1 );
    stepNames.push_back( edgedName );

    // Colliders
    std::string colliderName = Stringf( "%d: Phys. Colliders", stepIndex + 2 );
    stepNames.push_back( colliderName );

    return stepNames;
}


void MapWindow::UpdateChild( float deltaSeconds ) {
    Map*& theMap = m_mapPerStep[m_stepIndex];
    m_windowName = theMap->GetMapName();

    theMap->Update( deltaSeconds );
    g_theRenderer->BeginCamera( m_mapCamera );
    theMap->Render();
    g_theRenderer->EndCamera( m_mapCamera );

    void* mapView = m_mapCamera->GetRenderTarget()->GetShaderView();

    Vec2 contentMin = ImGui::GetWindowContentRegionMin();
    Vec2 contentMax = ImGui::GetWindowContentRegionMax();
    Vec2 contentSize = contentMax - contentMin;

    // Calculate pixel sizes for map
    IntVec2 mapSizeTiles = theMap->GetMapDimensions();

    Vec2 pixelsPerTile2 = contentSize / mapSizeTiles;
    float pixelsPerTile = Min( pixelsPerTile2.x, pixelsPerTile2.y ); // Tile size in pixels

    Vec2 mapSizePixels = pixelsPerTile * mapSizeTiles; // Map size in pixels

    AABB2 contentBounds = AABB2( contentMin, contentMax );
    AABB2 mapBounds = contentBounds.GetBoxWithin( mapSizePixels, ALIGN_BOTTOM_LEFT ); // Y is inverted.. actually means top left



    ImGui::Image( mapView, mapBounds.GetDimensions().GetAsImGui() ); // Map render

    //ImGui::GetForegroundDrawList()->AddRect( contentBounds.mins.GetAsImGui(), contentBounds.maxs.GetAsImGui(), 0xFFFF'FF00 );
    ImGui::GetForegroundDrawList()->AddRect( mapBounds.mins.GetAsImGui(), mapBounds.maxs.GetAsImGui(), 0xFFFF'FFFF );

    if( ImGui::IsItemHovered() ) {
        ImVec2 cursorPos = ImGui::GetMousePos();

        if( mapBounds.IsPointInside( cursorPos ) ) {
            Vec2 mapOrigin = mapBounds.mins;
            Vec2 cursorOffset = cursorPos - mapOrigin;

            Vec2 cursorTileFloat = cursorOffset / pixelsPerTile;
            IntVec2 cursorTileCoord = IntVec2( (int)cursorTileFloat.x, (int)cursorTileFloat.y );
            
            Vec2 tileMin = mapOrigin + (pixelsPerTile * cursorTileCoord);
            Vec2 tileMax = tileMin + Vec2( pixelsPerTile );

            Rgba tileHighlight = Rgba::GREEN;
            tileHighlight.a = .3f;

            unsigned char packedColor[4];
            tileHighlight.GetAsBytes( packedColor );
            ImU32 highlightColor = *(ImU32*)packedColor;

            ImGui::GetForegroundDrawList()->AddRectFilled( tileMin.GetAsImGui(), tileMax.GetAsImGui(), highlightColor );
        }
    }

/*  // Working image drawing
    ImVec2 imageMin = ImGui::GetItemRectMin();
    ImVec2 imageMax = ImGui::GetItemRectMax();

    Rgba tileHighlight = Rgba::GREEN;
    tileHighlight.a = .3f;

    unsigned char packedColor[4];
    tileHighlight.GetAsBytes( packedColor );
    ImU32 packedU32 = *(ImU32*)packedColor;

    ImGui::GetForegroundDrawList()->AddRectFilled( imageMin, imageMax, packedU32 );
*/
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
