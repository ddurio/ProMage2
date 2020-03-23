#include "Game/CreditsController.hpp"

#include "Engine/Core/Timer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Model.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView2D.hpp"

#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameStatePlay.hpp"


CreditsController::CreditsController( Actor* myActor ) :
    ActorController( myActor ) {
    // Materials
    g_theRenderer->GetOrCreateMaterial( m_grayscaleMatName );
    g_theRenderer->GetOrCreateMaterial( m_checkerMatName );

    Material* creditsMat = g_theRenderer->GetOrCreateMaterial( m_creditsMatName );
    creditsMat->SetShader( "BuiltIn/DevConsole" );

    // Camera
    m_creditsCamera = new Camera();
    m_creditsCamera->SetOrthoProjection( 10.f );

    TextureView2D* creditsView = g_theRenderer->GetOrCreateRenderTarget( m_creditsViewName );
    m_creditsCamera->SetRenderTarget( creditsView );

    // Model
    m_creditsModel = new Model();
    m_creditsModel->SetMaterial( m_creditsMatName );
    m_creditsModel->SetModelMatrix( Matrix44::IDENTITY );

    // Meshes
    BuildGuildhallMesh();
    BuildTitleMesh();
    BuildThanksMesh();
}


CreditsController::~CreditsController() {
    CLEAR_POINTER( m_creditsModel  );
    CLEAR_POINTER( m_creditsCamera );

    CLEAR_POINTER( m_guildhallMesh );
    CLEAR_POINTER( m_titleMesh );
    CLEAR_POINTER( m_thanksMesh );
}


void CreditsController::Update( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    if( !m_creditsAreRolling ) {
        return;
    }

    Material* creditsMat = g_theRenderer->GetOrCreateMaterial( m_creditsMatName );
    GPUMesh* creditsMesh = m_creditsModel->GetMesh();

    // Start with guildhallMesh
    if( creditsMesh == nullptr ) {
        m_fadeIn = true;
        m_numCycles = 2;

        creditsMat->SetTexture( TEXTURE_CREDTIS_GUILDHALL );
        m_creditsModel->SetMesh( m_guildhallMesh );
        return;
    }

    // Check if enough timer cycles have completed to transition
    // Guildhall --> Title --> Thanks
    if( m_fadeTimer->GetNumFinished() == m_numCycles + 1 ) {
        m_fadeOut = true;
    } else if( m_fadeTimer->GetNumFinished() > m_numCycles + 1 ) {
        m_fadeTimer->DecrementAll();
        m_fadeOut = false;
        m_fadeIn = true;

        BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );
        creditsMat->SetTexture( font->GetTexturePath() );

        if( creditsMesh == m_guildhallMesh ) {
            m_numCycles = 4;
            m_creditsModel->SetMesh( m_titleMesh );
        } else if( creditsMesh == m_titleMesh ) {
            m_numCycles = 4;
            m_creditsModel->SetMesh( m_thanksMesh );
        } else {
            g_theApp->HandleQuitRequested();
        }
    }
}


void CreditsController::RenderCredits() const {
    if( m_creditsModel->GetMesh() == nullptr ) {
        return;
    }

    // Apply grayscale effect to scene
    TextureView2D* sceneView = g_theRenderer->GetCurrentRenderTarget();
    Material* grayscaleMat = g_theRenderer->GetOrCreateMaterial( m_grayscaleMatName );
    g_theRenderer->ApplyEffect( sceneView, grayscaleMat );

    // Setup fade data for shader (1 float, 3 padding)
    float fadeFraction[4] = { 1.f }; // Fully on

    if( m_fadeIn || m_fadeOut ) {
        float percentComplete = m_fadeTimer->GetNormalizedElapsedTime();
        percentComplete = Clamp( percentComplete, 0.f, 1.f );

        if( m_fadeIn ) {
            fadeFraction[0] = percentComplete;
        } else if( m_fadeOut ) {
            fadeFraction[0] = 1 - percentComplete;
        }
    }

    // Push fade data to GPU (bound during DrawModel below)
    Material* creditsMat = g_theRenderer->GetOrCreateMaterial( m_creditsMatName );
    creditsMat->SetUBO( &fadeFraction, 4 * sizeof( float ) );

    // Render Credits to UI
    g_theRenderer->BeginCamera( m_creditsCamera );
    g_theRenderer->ClearRenderTarget( Rgba::CLEAR_BLACK );
    g_theRenderer->DrawModel( m_creditsModel );
    g_theRenderer->EndCamera( m_creditsCamera );

    // Apply checkered effect
    g_theRenderer->ApplyEffect( m_creditsCamera->GetRenderTarget(), m_checkerMatName, sceneView );
}


bool CreditsController::InteractWithActor( Actor* instigator ) {
    UNUSED( instigator );

    if( m_creditsAreRolling ) {
        return true;
    }

    m_creditsAreRolling = true;
    m_fadeIn = true;

    // Setup fade
    Clock* gameClock = g_theGame->GetGameClock();
    m_fadeTimer = new Timer( gameClock );
    m_fadeTimer->Start( m_fadeSeconds );

    // Pause gameState
    GameStatePlay* playState = (GameStatePlay*)g_theGame->GetGameState();
    Clock* stateClock = playState->GetStateClock();
    stateClock->Pause();

    // Register to be renderer
    playState->RegisterCreditsController( this );

    return true;
}


void CreditsController::BuildGuildhallMesh() {
    AABB2 cameraBounds = m_creditsCamera->GetBounds();

    CPUMesh builder;
    builder.AddQuad( cameraBounds );

    CLEAR_POINTER( m_guildhallMesh );
    m_guildhallMesh = new GPUMesh( g_theRenderer );
    m_guildhallMesh->CopyVertsFromCPUMesh( &builder );
}


void CreditsController::BuildTitleMesh() {
    CPUMesh builder;

    AABB2 cameraBounds = m_creditsCamera->GetBounds();
    BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );

    VertexList verts;
    font->AddVertsForTextInBox2D( verts, cameraBounds, 1.5f, "Adventure",                    Rgba::ORGANIC_BLUE, 1.f, Vec2( 0.5f, 0.75f ), TEXT_DRAW_SHRINK_TO_FIT );
    font->AddVertsForTextInBox2D( verts, cameraBounds, .7f,  "Danny Durio",                  Rgba::ORGANIC_BLUE, 1.f, Vec2( 0.5f, 0.5f  ), TEXT_DRAW_SHRINK_TO_FIT );
    font->AddVertsForTextInBox2D( verts, cameraBounds, .5f,  "SMU Guildhall C28 Programmer", Rgba::ORGANIC_BLUE, 1.f, Vec2( 0.5f, 0.4f  ), TEXT_DRAW_SHRINK_TO_FIT );
    builder.AddVertexArray( verts );

    CLEAR_POINTER( m_titleMesh );
    m_titleMesh = new GPUMesh( g_theRenderer );
    m_titleMesh->CopyVertsFromCPUMesh( &builder );
}


void CreditsController::BuildThanksMesh() {
    CPUMesh builder;

    AABB2 cameraBounds = m_creditsCamera->GetBounds();
    BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );

    VertexList verts;
    font->AddVertsForTextInBox2D( verts, cameraBounds, 1.0f, "Special Thanks",     Rgba::BLACK,        1.f, Vec2( 0.5f, 0.75f ), TEXT_DRAW_SHRINK_TO_FIT );
    font->AddVertsForTextInBox2D( verts, cameraBounds, 0.6f, "Jackie Durio",       Rgba::ORGANIC_BLUE, 1.f, Vec2( 0.5f, 0.6f ),  TEXT_DRAW_SHRINK_TO_FIT );
    font->AddVertsForTextInBox2D( verts, cameraBounds, 0.6f, "Squirrel Eiserloh",  Rgba::ORGANIC_BLUE, 1.f, Vec2( 0.5f, 0.5f ),  TEXT_DRAW_SHRINK_TO_FIT );
    font->AddVertsForTextInBox2D( verts, cameraBounds, 0.6f, "Chris Forseth",      Rgba::ORGANIC_BLUE, 1.f, Vec2( 0.5f, 0.4f ),  TEXT_DRAW_SHRINK_TO_FIT );
    font->AddVertsForTextInBox2D( verts, cameraBounds, 0.6f, "SMU Guildhall C28",  Rgba::ORGANIC_BLUE, 1.f, Vec2( 0.5f, 0.3f ),   TEXT_DRAW_SHRINK_TO_FIT );
    builder.AddVertexArray( verts );

    CLEAR_POINTER( m_thanksMesh );
    m_thanksMesh = new GPUMesh( g_theRenderer );
    m_thanksMesh->CopyVertsFromCPUMesh( &builder );
}
