#include "Game/GameStatePlay.hpp"

#include "Engine/Core/DebugDraw.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/Profiler.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RNG.hpp"
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

#include "Game/Actor.hpp"
#include "Game/ActorController.hpp"
#include "Game/ActorDef.hpp"
#include "Game/CreditsController.hpp"
#include "Game/Game.hpp"
#include "Game/GameInput.hpp"
#include "Game/Inventory.hpp"
#include "Game/ItemDef.hpp"
#include "Game/Map.hpp"
#include "Game/MapDef.hpp"
#include "Game/TileDef.hpp"
#include "Game/TopDownFollowCamera.hpp"
#include "Game/UIButton.hpp"
#include "Game/UILabel.hpp"
#include "Game/UIWidget.hpp"


enum StairType {
    STAIRS_UP,
    STAIRS_DOWN
};


GameStatePlay::GameStatePlay() {
    // Cameras
    m_uiCamera->SetOrthoProjection( 10.f );

    m_gameCamera = new TopDownFollowCamera( nullptr );
    m_gameCamera->SetOrthoProjection( 10.f );

    m_debugCamera = new Camera(); // Projection set in SetupDebugCamera

    // Resources
    BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );
    g_theRenderer->BindTexture( font->GetTexturePath() );

    BuildLoadingMesh();
}


GameStatePlay::~GameStatePlay() {
    CLEAR_POINTER( m_map );
    CLEAR_POINTER( m_gameInput );
    CLEAR_POINTER( m_debugCamera );
    CLEAR_POINTER( m_pauseUI );
    Map::ResetMaterialCreated();

    g_theEventSystem->Unsubscribe( "playerDeath", this, &GameStatePlay::HandlePlayerDeath );
    g_theEventSystem->Unsubscribe( "goToFloor", this, &GameStatePlay::Command_GoToFloor );
}


void GameStatePlay::Startup() {
    m_stateClock = new Clock( g_theGame->GetGameClock() );

    m_gameInput = new GameInput();
    m_gameInput->Startup();

    RenderContext* renderer = g_theRenderer;

    // Check for debugFlag
    std::string debugFlagStr = g_theGameConfigBlackboard.GetValue( "debugFlags", "" );
    Strings debugFlags = SplitStringOnDelimeter( debugFlagStr, ',', false );

    if( EngineCommon::VectorContains( debugFlags, std::string("NO_PRELOAD_TEXTURES") ) ) {
        renderer = nullptr;
    }

    // PreLoad all game assets
    SpriteSheet::Initialize( DATA_PAPER_DOLL_SPRITES, "SpriteSheet", renderer );
    SpriteAnimDef::Initialize( DATA_PAPER_DOLL_ANIMS, "SpriteAnim" );
    IsoSpriteAnimDef::Initialize( DATA_PAPER_DOLL_ISO_ANIMS, "IsoSpriteAnim" );

    SpriteSheet::Initialize( DATA_INVENTORY_SPRITES, "SpriteSheet", renderer );
    SpriteAnimDef::Initialize( DATA_INVENTORY_SPRITES, "SpriteAnim" );

    ActorDef::LoadFromFile( DATA_ACTOR_DEFS, "ActorDef" );
    ItemDef::LoadFromFile( DATA_ITEM_DEFS, "ItemDef" );

    TileDef::InitializeTileDefs();
    MapDef::InitializeMapDefs();
    ParseMapProgression();

    m_floorZeroSeed = g_RNG->GetRandomSeed();
    m_mapRNG = new RNG( m_floorZeroSeed );
    GoToFloor( 0, STAIRS_DOWN );

    g_theEventSystem->Subscribe( "playerDeath", this, &GameStatePlay::HandlePlayerDeath );
    g_theEventSystem->Subscribe( "goToFloor", this, &GameStatePlay::Command_GoToFloor );

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


    if( m_deathTimer->HasFinshed() ) {
        EventArgs args;
        HandlePlayerDeath( args );
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

    Camera* activeCamera = GetActiveCamera();
    g_theRenderer->BeginCamera( activeCamera );

    g_theRenderer->ClearRenderTarget( Rgba::BLACK );
    g_theRenderer->BindShader( nullptr );

    m_map->Render();

    if( m_isDebugging ) {
        g_thePhysicsSystem->RenderDebug();
        g_theDebugger->RenderWorld( activeCamera );
    }

    g_theRenderer->EndCamera( activeCamera );

    if( m_credits != nullptr ) {
        // Uses its own camera
        m_credits->RenderCredits();
    }


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
    } else if( keyCode == KB_F2 ) { // Cheat go down a floor
        ChangeFloorsDown();
        return true;
    } else if( keyCode == KB_F3 ) { // Cheat go up a floor
        ChangeFloorsUp();
        return true;
    } else if( keyCode == KB_F4 ) { // Cheat make player invulnerable
        Actor* player = m_map->GetPlayer();
        player->SetKillable( !player->IsKillable() );
    } else if( keyCode == KB_F5 ) { // Cheat give player gold
        Actor* player = m_map->GetPlayer();
        Inventory* playerInv = player->GetInventory();
        playerInv->TransferMoney( 10000 );
    } else if( keyCode == KB_F6 ) { // Cheat use debug camera
        m_useDebugCamera = !m_useDebugCamera;
    } else if( keyCode == KB_F || keyCode == KB_SPACE ) { // Take the Stairs
        Actor* player = m_map->GetPlayer();

        if( player != nullptr && player->IsAlive() ) {
            ActorController* controller = player->GetController();

            if( controller != nullptr && controller->TakeClosestStairs() ) {
                return true;
            }
        }
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


void GameStatePlay::ChangeFloorsDown() {
    unsigned int floorIndex = m_currentFloor + 1;
    GoToFloor( floorIndex, STAIRS_UP );
}


void GameStatePlay::ChangeFloorsUp() {
    unsigned int floorIndex = m_currentFloor - 1;
    GoToFloor( floorIndex, STAIRS_DOWN );
}


void GameStatePlay::RegisterCreditsController( CreditsController* controller ) {
    m_credits = controller;
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


Camera* GameStatePlay::GetActiveCamera() const {
    if( m_useDebugCamera || m_currentFloor == 22 || m_currentFloor == 222 ) {
        return m_debugCamera;
    } else {
        return m_gameCamera;
    }
}


std::string GameStatePlay::GetFloorTypeFromIndex() const {
    int numKeys = (int)m_mapTypeFloors.size();

    if( numKeys == 0 ) {
        return "Cave";
    }

    int keyIndex = 0;

    for( keyIndex; keyIndex < numKeys; keyIndex++ ) {
        const unsigned int& floorNumber = m_mapTypeFloors[keyIndex];

        if( m_currentFloor < floorNumber ) {
            break;
        }
    }

    keyIndex = Max( --keyIndex, 0 );
    return m_mapTypeNames[keyIndex];
}


void GameStatePlay::ParseMapProgression() {
    XmlDocument doc;
    const XMLElement& root = ParseXMLRootElement( DATA_PROGRESSION, doc );

    const XMLElement* mapEle = root.FirstChildElement( "Maps" );
    const XMLElement* keyEle = mapEle->FirstChildElement( "KeyFrame" );

    while( keyEle != nullptr ) {
        int floorIndex  = ParseXMLAttribute( *keyEle, "floor",     0 );
        std::string mapType = ParseXMLAttribute( *keyEle, "mapType", "" );
        GUARANTEE_OR_DIE( mapType != "", "(GameStatePlay) Map progression missing required attribute 'mapType'" );

        m_mapTypeFloors.push_back( floorIndex );
        m_mapTypeNames.push_back( mapType );

        keyEle = keyEle->NextSiblingElement( "KeyFrame" );
    }
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
    g_theRenderer->GetOrCreateRenderTarget( m_pauseViewName );
    g_theRenderer->GetOrCreateMaterial( m_grayscaleMatName );
    g_theRenderer->GetOrCreateMaterial( m_pauseMatName );
    m_pauseUBO = new UniformBuffer( g_theRenderer );
}


void GameStatePlay::GoToFloor( unsigned int newFloorIndex, StairType stairType ) {
    m_currentFloor = newFloorIndex;

    m_mapRNG->SetSeed( m_floorZeroSeed + m_currentFloor );
    m_mapRNG->SetPosition( 0 );

    // Choose floor type
    std::string floorType = GetFloorTypeFromIndex();

    // Setup new map
    Map* nextMap = new Map( Stringf( "Floor%d", m_currentFloor ), floorType, m_mapRNG, newFloorIndex );
    nextMap->Startup();

    if( m_map != nullptr ) {
        Actor* player = m_map->GetPlayer();
        m_map->RemovePlayerFromMap( player );
        nextMap->AddPlayerToMap( player );

        IntVec2 stairCoords = nextMap->GetTileCoordsForStairs( stairType );

        if( stairCoords != IntVec2::NEGONE ) {
            Vec2 stairsWorldCoords = Vec2( stairCoords ) + Vec2( 0.5f );
            player->SetWorldPosition( stairsWorldCoords );
        }

        m_map->Shutdown();
        CLEAR_POINTER( m_map );
    }

    m_map = nextMap;
    SetupDebugCamera();
}


bool GameStatePlay::Command_GoToFloor( EventArgs& args ) {
    int floorIndex = args.GetValue( "number", 0 );
    GoToFloor( floorIndex, STAIRS_UP );

    return true;
}


void GameStatePlay::SetupDebugCamera() {
    IntVec2 mapDims = m_map->GetMapDimensions();
    Vec2 halfDims = mapDims / 2.f;

    m_debugCamera->SetOrthoProjection( (float)mapDims.y );
    m_debugCamera->Translate( halfDims );
}


bool GameStatePlay::HandlePlayerDeath( EventArgs& args ) {
    UNUSED( args );

    if( m_deathTimer == nullptr ) {
        m_deathTimer = new Timer( m_stateClock );
        m_deathTimer->Start( 3.f );
    } else {
        CLEAR_POINTER( m_deathTimer );

        Actor* player0 = m_map->GetPlayer();
        player0->Revive();

        if( m_currentFloor != 0 ) {
            GoToFloor( 0, STAIRS_DOWN );
        }
    }

    return false;
}
