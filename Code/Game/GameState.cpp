#include "Game/GameState.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/Game.hpp"


GameState::GameState() {
    m_uiCamera = new Camera();
    m_gameCamera = new Camera();
}


GameState::~GameState() {
    CLEAR_POINTER( m_uiCamera );
    CLEAR_POINTER( m_gameCamera );

    CLEAR_POINTER( m_loadingMesh );
    CLEAR_POINTER( m_loadedMesh );
}


Camera* GameState::GetUICamera() const {
    return m_uiCamera;
}


Camera* GameState::GetGameCamera() const {
    return m_gameCamera;
}


void GameState::BuildLoadingMesh() {
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


void GameState::BuildLoadedMesh() {
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
