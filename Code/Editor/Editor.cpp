#if defined(_EDITOR)
#include "Editor/Editor.hpp"

#include "Editor/EditorMapDef.hpp"
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
    m_loadState = LOAD_STATE_INIT;

    BuildLoadingMesh();
}


Editor::~Editor() {
    CLEAR_POINTER( m_uiCamera );
    CLEAR_POINTER( m_loadingMesh );
    CLEAR_POINTER( m_loadedMesh );
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

    g_theEventSystem->Subscribe( "saveAll", &EditorMapDef::SaveAllToXml );

    // Setup Editor
    m_mapWindow  = new MapWindow(  Vec2( 0.65f, 0.88f ), Vec2( 0.f, 0.83f ) );
    m_stepWindow = new StepWindow( Vec2( 0.65f, 0.1f ) );
    m_xmlWindow  = new XmlWindow(  Vec2( 0.35f, 0.98f ), Vec2( 1.f, 0.f ) );

    // Setup Style
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Header] = Rgba::ORGANIC_GRAY.GetAsImGui();

    BuildLoadedMesh();

    m_loadState = LOAD_STATE_READY;
}


void Editor::Shutdown() {
    int numResults = (int)m_customResultIndex.size();

    for( int resultIndex = 0; resultIndex < numResults; resultIndex++ ) {
        MapGenStep::RemoveCustomResult( m_customResultIndex[resultIndex] );
    }

    ActorDef::DestroyDefs();
    ItemDef::DestroyDefs();
    TileDef::DestroyDefs();

    g_theEventSystem->Unsubscribe( "saveAll", &EditorMapDef::SaveAllToXml );
    EditorMapDef::DestroyDefs();
}


bool Editor::HandleKeyPressed( unsigned char keyCode ) {
    if( m_loadState == LOAD_STATE_READY ) {
        if( keyCode == KB_SPACE ) { // Continue to Play
            m_loadState = LOAD_STATE_DONE;
            return true;
        }
    }

    // ThesisFIXME: Implement key pressed
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
    if( !UpdateIsLoaded() ) {
        return;
    }

    float deltaSeconds = m_editorClock.GetDeltaTime();

    UpdateMainMenu();
    m_mapWindow->Update( deltaSeconds );
    m_stepWindow->Update( deltaSeconds );
    m_xmlWindow->Update( deltaSeconds );

    if( m_demoIsShown ) {
        ImGui::ShowDemoWindow();
    }
}


void Editor::Render() const {
    if( !RenderIsLoaded() ) {
        return;
    }

    m_mapWindow->Render();
    m_stepWindow->Render();
    m_xmlWindow->Render();
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


void Editor::Execute() {
    Startup();
}


// PRIVATE -------------------------------
bool Editor::UpdateIsLoaded() {
    if( m_loadState == LOAD_STATE_DONE ) {
        return true;
    }
    
    if( m_loadState == LOAD_STATE_INIT ) { // Loading

    } else if( m_loadState == LOAD_STATE_READY ) { // Loaded

    }

    return false;
}


bool Editor::RenderIsLoaded() const {
    if( m_loadState == LOAD_STATE_DONE ) {
        return true;
    }

    g_theRenderer->BeginCamera( m_uiCamera );

    BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );
    g_theRenderer->BindTexture( font->GetTexturePath() );

    if( m_loadState == LOAD_STATE_INIT ) { // LOADING
        g_theRenderer->ClearRenderTarget( Rgba::CYAN );
        g_theRenderer->DrawMesh( m_loadingMesh );
    } else if( m_loadState == LOAD_STATE_READY ) { // LOADED
        g_theRenderer->ClearRenderTarget( Rgba::YELLOW );
        g_theRenderer->DrawMesh( m_loadedMesh );
    }

    g_theRenderer->EndCamera( m_uiCamera );
    return false;
}


void Editor::BuildLoadingMesh() {
    if( m_loadingMesh != nullptr ) {
        return; // Already built
    }

    BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );
    CPUMesh builder;
    VertexList verts;
    AABB2 cameraBounds = m_uiCamera->GetBounds();
    Vec2 cameraDimensions = cameraBounds.GetDimensions();

    // Create text verts
    font->AddVeretsForTextInBox2D( verts, cameraBounds, cameraDimensions.y * 0.1f, "Loading...", Rgba::BLACK );
    builder.AddVertexArray( verts );

    // Create Mesh
    m_loadingMesh = new GPUMesh( g_theRenderer );
    m_loadingMesh->CopyVertsFromCPUMesh( &builder );
}


void Editor::BuildLoadedMesh() {
    if( m_loadedMesh != nullptr ) {
        return; // Already built
    }

    BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );
    CPUMesh builder;
    VertexList verts;
    AABB2 cameraBounds = m_uiCamera->GetBounds();
    Vec2 cameraDimensions = cameraBounds.GetDimensions();

    // Create text verts
    font->AddVeretsForTextInBox2D( verts, cameraBounds, cameraDimensions.y * 0.1f, "Loaded", Rgba::BLACK );
    font->AddVeretsForTextInBox2D( verts, cameraBounds, cameraDimensions.y * 0.05f, "Press Space to Continue", Rgba::BLACK, 1.f, Vec2( 0.5f, 0.25f ) );
    builder.AddVertexArray( verts );

    // Create Mesh
    m_loadedMesh = new GPUMesh( g_theRenderer );
    m_loadedMesh->CopyVertsFromCPUMesh( &builder );
}


void Editor::UpdateMainMenu() {
    ImGui::BeginMainMenuBar();
    
    if( ImGui::BeginMenu( "File" ) ) {
        ImGui::MenuItem( "New Map Type" );
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}


bool Editor::ToggleDemo( EventArgs& args ) {
    UNUSED( args );
    m_demoIsShown = !m_demoIsShown;

    return true;
}

#endif
