#include "Game/GameStatePlay.hpp"

#include "Engine/Core/DebugDraw.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/IsoSpriteAnimDef.hpp"
#include "Engine/Renderer/Model.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDef.hpp"
#include "Engine/Renderer/TextureView2D.hpp"
#include "Engine/Renderer/UniformBuffer.hpp"

#include "Game/ActorDef.hpp"
#include "Game/Game.hpp"
#include "Game/GameInput.hpp"
#include "Game/ItemDef.hpp"
#include "Game/Map.hpp"
#include "Game/MapDef.hpp"
#include "Game/TileDef.hpp"
#include "Game/TopDownFollowCamera.hpp"
#include "Game/UIButton.hpp"
#include "Game/UILabel.hpp"
#include "Game/UIWidget.hpp"


GameStatePlay::GameStatePlay() {
    // Cameras
    m_uiCamera->SetOrthoProjection( 10.f );

    m_gameCamera = new TopDownFollowCamera( nullptr );
    m_gameCamera->SetOrthoProjection( 10.f );

    // Resources
    BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );
    g_theRenderer->BindTexture( font->GetTexturePath() );

    BuildLoadingMesh();
}


GameStatePlay::~GameStatePlay() {
    CLEAR_POINTER( m_map );
    CLEAR_POINTER( m_gameInput );
    CLEAR_POINTER( m_pauseUI );
}


void GameStatePlay::Startup() {
    m_stateClock = new Clock( g_theGame->GetGameClock() );

    m_gameInput = new GameInput();
    m_gameInput->Startup();

    SpriteSheet::Initialize( DATA_PAPER_DOLL_SPRITES, "SpriteSheet" );
    SpriteAnimDef::Initialize( DATA_PAPER_DOLL_ANIMS, "SpriteAnim" );
    IsoSpriteAnimDef::Initialize( DATA_PAPER_DOLL_ISO_ANIMS, "IsoSpriteAnim" );

    SpriteSheet::Initialize( DATA_INVENTORY_SPRITES, "SpriteSheet" );
    SpriteAnimDef::Initialize( DATA_INVENTORY_SPRITES, "SpriteAnim" );

    Definition<Actor>::Initialize( DATA_ACTOR_DEFS, "ActorDef" );
    Definition<Item>::Initialize( DATA_ITEM_DEFS, "ItemDef" );
    TileDef::InitializeTileDefs();
    MapDef::InitializeMapDefs();

    m_map = new Map( "DFS1FIXME", "Island" ); // DFS1FIXME: Set correct map name
    m_map->Startup();

    Entity* player0 = m_map->GetPlayer();
    ((TopDownFollowCamera*)m_gameCamera)->SetFollowTarget( player0 );

    BuildPauseUI();
    BuildLoadedMesh();
}


void GameStatePlay::Shutdown() {
    m_map->Shutdown();
    m_gameInput->Shutdown();
}


void GameStatePlay::Update() {
    if( m_loadState != LOAD_STATE_DONE ) {
        switch( m_loadState ) {
            case(LOAD_STATE_PRE_INIT): {
                m_loadState = LOAD_STATE_INIT;
                return;
            } case(LOAD_STATE_INIT): {
                Startup();
                m_loadState = LOAD_STATE_READY;
                return;
            } case(LOAD_STATE_READY): {
                // Could flash / change alpha of text here
                return;
            }
        }
    }

    float deltaSeconds = m_stateClock->GetDeltaTime();

    bool inputPaused = m_gameInput->IsPaused();
    // DFS1FIXME: Fix pause logic, completely disabled in gameInput

    if( m_isPaused && !inputPaused ) { // Unpausing but wait for fade
        if( !m_fadeOut ) {
            m_fadeTimer->Start( m_fadeSeconds );
            m_fadeOut = true;
        }

        if( m_fadeTimer->HasFinshed() ) {
            m_isPaused = false;
        }
    } else if( m_isPaused || inputPaused ) { // Start pause or during pause
        deltaSeconds = 0.f;
        m_pauseUI->UpdateBounds( m_uiCamera->GetBounds() );

        if( m_uiCamera->GetRenderTarget() == nullptr ) {
            TextureView2D* pauseView = g_theRenderer->GetOrCreateTextureView2D( m_pauseViewName );
            m_uiCamera->SetRenderTarget( pauseView );
        }

        if( m_fadeTimer == nullptr ) {
            m_fadeTimer = new Timer();
            m_fadeTimer->Start( m_fadeSeconds );

            m_fadeOut = false;
            m_isPaused = true;
        }
    } else {
        CLEAR_POINTER( m_fadeTimer );
    }

    m_map->Update( deltaSeconds );
    g_thePhysicsSystem->Update( deltaSeconds );
    g_theDebugger->Update( deltaSeconds );
    ((TopDownFollowCamera*)m_gameCamera)->Update( deltaSeconds ); // Must be after map entities are updated

    // Update last to clear flags
    m_gameInput->Update( deltaSeconds );
}


void GameStatePlay::Render() {
    if( m_loadState != LOAD_STATE_DONE ) {
        g_theRenderer->BeginCamera( m_uiCamera );


        BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );
        g_theRenderer->BindTexture( font->GetTexturePath() );

        if( m_loadState == LOAD_STATE_INIT ) { // LOADING
            g_theRenderer->ClearRenderTarget( Rgba::CYAN );
            g_theRenderer->DrawMesh( m_loadingMesh );
            g_theRenderer->EndCamera( m_uiCamera );
            return;
        } else if( m_loadState == LOAD_STATE_READY ) { // LOADED
            g_theRenderer->ClearRenderTarget( Rgba::YELLOW );
            g_theRenderer->DrawMesh( m_loadedMesh );
            g_theRenderer->EndCamera( m_uiCamera );
            return;
        }
    }

    g_theRenderer->BeginCamera( m_gameCamera );

    g_theRenderer->ClearRenderTarget( Rgba::BLACK );
    g_theRenderer->BindShader( nullptr );

    m_map->Render();

    if( m_isDebugging ) {
        g_thePhysicsSystem->RenderDebug();
        g_theDebugger->RenderWorld( m_gameCamera );
    }

    g_theRenderer->EndCamera( m_gameCamera );



    if( m_isPaused ) {
        TextureView2D* sceneView = g_theRenderer->GetCurrentRenderTarget();
        Material* grayscaleMat = g_theRenderer->GetOrCreateMaterial( m_grayscaleMatName );
        g_theRenderer->ApplyEffect( sceneView, grayscaleMat );

        g_theRenderer->BeginCamera( m_uiCamera );
        m_pauseUI->Render();
        g_theRenderer->EndCamera( m_uiCamera );

        float fadeFraction[4];
        fadeFraction[0] = ClampFloat( m_fadeTimer->GetNormalizedElapsedTime(), 0.f, 1.f );
        fadeFraction[0] = m_fadeOut ? 1 - fadeFraction[0] : fadeFraction[0];

        m_pauseUBO->CopyCPUToGPU( &fadeFraction, 4 * sizeof( float ) );
        g_theRenderer->BindUBO( m_pauseUBO, UBO_SLOT_USER_MATERIAL );

        g_theRenderer->ApplyEffect( m_uiCamera->GetRenderTarget(), m_pauseMatName, sceneView ); // Checker fade
    }
}


bool GameStatePlay::HandleKeyPressed( unsigned char keyCode ) {
    if( m_loadState == LOAD_STATE_READY ) {
        if( keyCode == KB_SPACE ) { // Continue to Play
            m_loadState = LOAD_STATE_DONE;
            return true;
        }
    }

    if( keyCode == KB_F1 ) {
        m_isDebugging = !m_isDebugging;
        return true;
    }

    return m_gameInput->HandleKeyPressed( keyCode );
}


bool GameStatePlay::HandleKeyReleased( unsigned char keyCode ) {
    return m_gameInput->HandleKeyReleased( keyCode );
}


bool GameStatePlay::HandleMouseButton( MouseEvent event, float scrollAmount /*= 0.f */ ) {
    if( m_isPaused ) {
        return m_pauseUI->HandleMouseButton( event, scrollAmount );
    }

    return m_gameInput->HandleMouseButton( event, scrollAmount );
}


bool GameStatePlay::HandleQuitRequested() {
    return false;
}


GameInput* GameStatePlay::GetGameInput() const {
    return m_gameInput;
}


bool GameStatePlay::Command_PauseGame( EventArgs& args ) {
    GameStatePlay* state = (GameStatePlay*)g_theGame->GetGameState();
    bool shouldPause = args.GetValue( "paused", false );

    g_theDevConsole->PrintString( Stringf("(Pause) Current: %s, Should: %s", state->m_isPaused ? "true" : "false", shouldPause ? "true" : "false"), DevConsole::CHANNEL_INFO );

    if( state->m_isPaused != shouldPause ) {
        state->m_gameInput->HandleKeyPressed( 0x1B ); // Fake pressing escape
    }

    return false;
}


void GameStatePlay::BuildPauseUI() {
    m_pauseUI = new UIWidget( ALIGN_CENTER, Vec2::ZERO, Vec2::ONE, Vec2::ZERO );
    m_pauseUI->SetColor( Rgba::CLEAR );

    Vec2 buttonSize = Vec2( 0.4f, 0.1f );

    UILabel* title = new UILabel( Vec2( 0.5f, 0.75f ), Vec2::ZERO, buttonSize, Vec2::ZERO, "Paused" );
    title->SetColor( Rgba::BLACK );
    m_pauseUI->AddChild( title );

    // Resume
    UIButton* resumeButton = new UIButton( Vec2( 0.5f, 0.5f ), Vec2::ZERO, buttonSize, Vec2::ZERO, "pauseGame paused=false" );
    resumeButton->SetColor( Rgba::FLAT );
    g_theEventSystem->SubscribeEventCallbackFunction( "pauseGame", Command_PauseGame );
    m_pauseUI->AddChild( resumeButton );

    UILabel* resumeLabel = new UILabel( ALIGN_CENTER, Vec2::ZERO, Vec2::ONE, Vec2::ZERO, "Resume" );
    resumeLabel->SetColor( Rgba::GRAY );
    resumeButton->AddChild( resumeLabel );

    // Quit
    UIButton* quitButton = new UIButton( Vec2( 0.5f, 0.25f ), Vec2::ZERO, buttonSize, Vec2::ZERO, "quit" );
    quitButton->SetColor( Rgba::FLAT );
    m_pauseUI->AddChild( quitButton );

    UILabel* quitLabel = new UILabel( ALIGN_CENTER, Vec2::ZERO, Vec2::ONE, Vec2::ZERO, "Quit" );
    quitLabel->SetColor( Rgba::GRAY );
    quitButton->AddChild( quitLabel );

    // Pause Shader
    g_theRenderer->GetNewRenderTarget( m_pauseViewName );
    g_theRenderer->GetOrCreateMaterial( m_grayscaleMatName );
    g_theRenderer->GetOrCreateMaterial( m_pauseMatName );
    m_pauseUBO = new UniformBuffer( g_theRenderer );
}
