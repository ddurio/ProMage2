#if defined(_EDITOR)
#include "Editor/Editor.hpp"

#include "Editor/EditorMapDef.hpp"
#include "Editor/ImGuiUtils.hpp"
#include "Editor/MapWindow.hpp"
#include "Editor/StepWindow.hpp"
#include "Editor/XmlWindow.hpp"

#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/IsoSpriteAnimDef.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDef.hpp"
#include "Engine/Utils/NamedStrings.hpp"

#include "Game/Actor.hpp"
#include "Game/ActorDef.hpp"
#include "Game/Item.hpp"
#include "Game/ItemDef.hpp"
#include "Game/MapGen/GenSteps/MapGenStep.hpp"
#include "Game/MapGen/GenSteps/MGS_CustomDef.hpp"
#include "Game/MapGen/GenSteps/MotifDef.hpp"
#include "Game/MapGen/Map/TileDef.hpp"


JobSystem* g_theJobs = new JobSystem();


Editor::Editor() :
    Job( g_theJobs, JOB_CATEGORY_GENERIC ) {
    SetAutoDestroy( false );

    m_uiCamera = new Camera();
    m_uiCamera->SetOrthoProjection( 10.f );

    BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );
    g_theRenderer->BindTexture( font->GetTexturePath() );

    g_theEventSystem->Subscribe( "guiDemo", this, &Editor::ToggleDemo );

    g_theEditor = this;
    g_theJobs->Startup();
    g_theJobs->StartJob( this ); // Calls startup on another thread
}


Editor::~Editor() {
    CLEAR_POINTER( m_uiCamera );
    CLEAR_POINTER( m_mapWindow );
    CLEAR_POINTER( m_stepWindow );
    CLEAR_POINTER( m_xmlWindow );

    Map::ResetMaterialCreated();

    g_theEventSystem->Unsubscribe( "guiDemo", this, &Editor::ToggleDemo );
}


void Editor::Startup() {
    RenderContext* renderer = g_theRenderer;

    // Check for debugFlag
    std::string debugFlagStr = g_theGameConfigBlackboard.GetValue( "debugFlags", "" );
    Strings debugFlags = SplitStringOnDelimeter( debugFlagStr, ',', false );

    if( EngineCommon::VectorContains( debugFlags, std::string("NO_PRELOAD_TEXTURES") ) ) {
        renderer = nullptr;
    }

    // Load asset files
    SpriteSheet::Initialize( DATA_PAPER_DOLL_SPRITES, "SpriteSheet", renderer );
    SpriteAnimDef::Initialize( DATA_PAPER_DOLL_ANIMS, "SpriteAnim" );
    IsoSpriteAnimDef::Initialize( DATA_PAPER_DOLL_ISO_ANIMS, "IsoSpriteAnim" );

    ActorDef::LoadFromFile( DATA_ACTOR_DEFS, "ActorDef" );
    ItemDef::LoadFromFile( DATA_ITEM_DEFS, "ItemDef" );
    TileDef::LoadFromFile( DATA_TILE_DEFS, "TileDefinition" );

    m_customResultIndex.push_back( Actor::SetupSpawnActorMGS() );
    m_customResultIndex.push_back( Item::SetupSpawnItemMGS() );
    m_customResultIndex.push_back( EditorMapDef::SetupChangeTileMGS() );
    MotifDef::LoadFromFile( DATA_MOTIF_DEFS, "Motif" );
    MGS_CustomDef::LoadFromFile( DATA_CUSTOM_STEPS, "Custom" );
    EditorMapDef::LoadFromFile( DATA_MAP_DEFS, "MapDefinition" );

    g_theEventSystem->Subscribe( EVENT_EDITOR_SAVE_MAPS, &EditorMapDef::SaveAllToXml );

    // Setup Editor
    m_mapWindow  = new MapWindow(  Vec2( 0.65f, 0.88f ), Vec2( 0.f, 0.83f ) );
    m_stepWindow = new StepWindow( Vec2( 0.65f, 0.1f ) );
    m_xmlWindow  = new XmlWindow(  Vec2( 0.35f, 0.98f ), Vec2( 1.f, 0.f ) );

    // Setup Style
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Header] = Rgba::ORGANIC_GRAY.GetAsImGui();

    m_editorIsLoading = false;
}


void Editor::Shutdown() {
    int numResults = (int)m_customResultIndex.size();

    for( int resultIndex = 0; resultIndex < numResults; resultIndex++ ) {
        MapGenStep::RemoveCustomResult( m_customResultIndex[resultIndex] );
    }

    ActorDef::DestroyDefs();
    ItemDef::DestroyDefs();
    TileDef::DestroyDefs();

    g_theEventSystem->Unsubscribe( EVENT_EDITOR_SAVE_MAPS, &EditorMapDef::SaveAllToXml );
    EditorMapDef::DestroyDefs();
}


bool Editor::HandleKeyPressed( unsigned char keyCode ) {
    // ThesisFIXME: Implement key pressed
    UNUSED( keyCode );
    return false;
}


bool Editor::HandleKeyReleased( unsigned char keyCode ) {
    UNUSED( keyCode );
    // ThesisFIXME: Implement key released
    return false;
}


bool Editor::HandleMouseButton( MouseEvent event, float scrollAmount /*= 0.f */ ) {
    return m_mapWindow->HandleMouseButton( event, scrollAmount );
}


void Editor::Update() {
    if( IsLoading() ) {
        UpdateLoading();
        return;
    }

    float deltaSeconds = m_editorClock.GetDeltaTime();

    m_mapWindow->Update( deltaSeconds );
    m_stepWindow->Update( deltaSeconds );
    bool regenTriggered = m_xmlWindow->Update( deltaSeconds );

    if( regenTriggered ) {
        return;
    }

    UpdateMenuBar();

    if( m_demoIsShown ) {
        ImGui::ShowDemoWindow();
    }
}


void Editor::Render() const {
    if( IsLoading() ) {
        RenderLoading();
        return;
    }

    m_mapWindow->Render();
    m_stepWindow->Render();
    m_xmlWindow->Render();
}


void Editor::SetMapLoadingState( bool isLoading ) {
    m_mapIsLoading = isLoading;
}


MapWindow* Editor::GetMapWindow() const {
    return m_mapWindow;
}


XmlWindow* Editor::GetXmlWindow() const {
    return m_xmlWindow;
}


const Clock* Editor::GetEditorClock() const {
    return &m_editorClock;
}


bool Editor::IsLoading() const {
    return (m_editorIsLoading || m_mapIsLoading);
}


// PRIVATE -------------------------------
void Editor::Execute() {
    Startup();
}


void Editor::UpdateLoading() {
    BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );

    AABB2 cameraBounds = m_uiCamera->GetBounds();
    Vec2 cameraDimensions = cameraBounds.GetDimensions();

    //int loadingIndex = GetIndexOverTime( 4, 0.25f );
    m_loadingIndex++;
    int numDots = (m_loadingIndex / 50) % 4;
    int maxGlyphs = 7 + numDots;

    // Create text verts
    m_loadingVerts.clear();
    font->AddVeretsForTextInBox2D(
        m_loadingVerts,
        cameraBounds,
        cameraDimensions.y * 0.1f,
        "Loading...",
        Rgba::ORGANIC_BLUE,
        1.f,
        ALIGN_CENTER,
        TEXT_DRAW_SHRINK_TO_FIT,
        maxGlyphs
    );
}


void Editor::UpdateMenuBar() {
    ImGui::BeginMainMenuBar();
    
    if( ImGui::BeginMenu( "File" ) ) {
        if( ImGui::BeginMenu( "New" ) ) {
            if( ImGui::BeginMenu( "Map Type" ) ) {
                ImGui::InputText( "newMapTypeName", &m_newMapType, ImGuiInputTextFlags_CharsNoBlank );
                RenderTileDropDown( "newMapTypeFill", m_newMapFill, "Fill Type", false, "" );

                if( ImGui::MenuItem( "New Map Type", "Ctrl + N" ) ) {
                    EditorMapDef::CreateNewMapDef( m_newMapType, m_newMapFill );
                    m_xmlWindow->TriggerMapGen( m_newMapType, -1, true );
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if( ImGui::MenuItem( "Open..." ) ) {
            // ThesisFIXME: Implement open mapDef file
        }

        ImGui::Separator();

        if( ImGui::MenuItem( "Save" ) ) {
            EventArgs args;
            args.SetValue( "filePath", m_mapDefFile );

            g_theEventSystem->FireEvent( EVENT_EDITOR_SAVE_MAPS, args );
        }

        if( ImGui::MenuItem( "Save As..." ) ) {
            // ThesisFIXME: Implement save as file dialog
        }

        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}


void Editor::RenderLoading() const {
    g_theRenderer->BeginCamera( m_uiCamera );

    BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );
    g_theRenderer->BindTexture( font->GetTexturePath() );

    g_theRenderer->ClearRenderTarget( Rgba::ORGANIC_GRAY );
    g_theRenderer->DrawVertexArray( m_loadingVerts );

    g_theRenderer->EndCamera( m_uiCamera );
}


bool Editor::ToggleDemo( EventArgs& args ) {
    UNUSED( args );
    m_demoIsShown = !m_demoIsShown;

    return true;
}

#endif
