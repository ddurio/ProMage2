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
#include "Game/MapGen/Map/TileDef.hpp"


Editor::Editor() {
    m_uiCamera = new Camera();
    m_uiCamera->SetOrthoProjection( 10.f );

    BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );
    g_theRenderer->BindTexture( font->GetTexturePath() );

    BuildLoadingMesh();
}


Editor::~Editor() {
    CLEAR_POINTER( m_uiCamera );
    CLEAR_POINTER( m_loadingMesh );
    CLEAR_POINTER( m_loadedMesh );
    CLEAR_POINTER( m_mapEditor );
    CLEAR_POINTER( m_stepEditor );
    CLEAR_POINTER( m_xmlEditor );

    Map::ResetMaterialCreated();
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
    EditorMapDef::LoadFromFile( DATA_MAP_DEFS, "MapDefinition" );

    // Setup Editor
    m_mapEditor = new MapWindow();
    m_stepEditor = new StepWindow( *m_mapEditor );
    m_xmlEditor = new XmlWindow();

    BuildLoadedMesh();
}


void Editor::Shutdown() {
    int numResults = (int)m_customResultIndex.size();

    for( int resultIndex = 0; resultIndex < numResults; resultIndex++ ) {
        MapGenStep::RemoveCustomResult( m_customResultIndex[resultIndex] );
    }

    ActorDef::DestroyDefs();
    ItemDef::DestroyDefs();
    TileDef::DestroyDefs();
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
    UNUSED( event );
    UNUSED( scrollAmount );
    // ThesisFIXME: Implement mouse event
    return false;
}


void Editor::Update() {
    if( !UpdateIsLoaded() ) {
        return;
    }

    float deltaSeconds = m_editorClock.GetDeltaTime();

    m_mapEditor->Update( deltaSeconds );
    m_stepEditor->Update( deltaSeconds );
    m_xmlEditor->Update( deltaSeconds );

    ImGui::ShowDemoWindow(); // ThesisFIXME: Needs to be disabled
}


void Editor::Render() const {
    if( !RenderIsLoaded() ) {
        return;
    }

    m_mapEditor->Render();
    m_stepEditor->Render();
    m_xmlEditor->Render();
}


Vec2 Editor::CreateWindow( const Vec2& normDimensions, const Vec2& alignment, const char* name /*= ""*/, ImGuiWindowFlags extraFlags /*= ImGuiWindowFlags_None */ ) const {
    // Setup Window
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove
                                 | ImGuiWindowFlags_NoResize
                                 | ImGuiWindowFlags_NoCollapse
                                 | ImGuiWindowFlags_NoSavedSettings
                                 | extraFlags;

    AABB2 clientBounds = g_theWindow->GetClientBoundsInverted();
    Vec2 clientDimensions = clientBounds.GetDimensions();

    Vec2 windowSize = normDimensions * clientDimensions;
    AABB2 windowBounds = clientBounds.GetBoxWithin( windowSize, alignment );
    Vec2 windowOrigin = Vec2( windowBounds.mins.x, -windowBounds.maxs.y );

    ImGui::SetNextWindowPos( ImVec2( windowOrigin.x, windowOrigin.y ), ImGuiCond_Always );
    ImGui::SetNextWindowSize( ImVec2( windowSize.x, windowSize.y ), ImGuiCond_Always );
    ImGui::Begin( name, nullptr, windowFlags );

    return windowSize;
}


// PRIVATE -------------------------------
bool Editor::UpdateIsLoaded() {
    if( m_loadState == LOAD_STATE_DONE ) {
        return true;
    }

    switch( m_loadState ) {
        case(LOAD_STATE_PRE_INIT): {
            m_loadState = LOAD_STATE_INIT;
            break;
        } case(LOAD_STATE_INIT): {
            Startup();
            m_loadState = LOAD_STATE_READY;
            break;
        } case(LOAD_STATE_READY): {
            // Could flash / change alpha of text here
            break;
        }
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

#endif
