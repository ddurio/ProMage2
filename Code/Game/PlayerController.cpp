#include "Game/PlayerController.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView2D.hpp"

#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/GameInput.hpp"


PlayerController::PlayerController( Actor* myActor, int playerID ) :
    ActorController( myActor ),
    m_playerIndex( playerID ) {
    m_gameInput = g_theGame->GetGameInput();

    g_theRenderer->GetNewRenderTarget( m_portraitViewName );
}


PlayerController::~PlayerController() {

}


void PlayerController::Update( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    // Movement
    Vec2 moveDir = m_gameInput->GetMovementDirection();
    SetMoveDir( moveDir );

    // Inventory
    bool toggleInventory = m_gameInput->WasInvtoryToggled();

    if( toggleInventory ) {
        ToggleInventory();
    }

    UpdateHUD();
}

void PlayerController::UpdateHUD() {
    // Setup Window
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

    AABB2 clientBounds = g_theWindow->GetClientBounds();
    Vec2 clientDimensions = clientBounds.GetDimensions();

    float windowWidth = 0.1f * clientDimensions.x;
    Vec2 windowSize = Vec2( windowWidth, -windowWidth );

    AABB2 window = clientBounds.GetBoxWithin( windowSize, Vec2::ZERO );
    Vec2 windowOrigin = Vec2( window.mins.x, window.maxs.y );

    ImGui::SetNextWindowPos( ImVec2( windowOrigin.x, windowOrigin.y ), ImGuiCond_Always );
    ImGui::SetNextWindowSize( ImVec2( windowWidth, windowWidth ), ImGuiCond_Always );
    ImGui::Begin( "Portrait", nullptr, windowFlags );

    // Setup Portrait
    TextureView2D* portraitView = g_theRenderer->GetOrCreateTextureView2D( m_portraitViewName );
    Camera* uiCamera = g_theGame->GetUICamera();
    uiCamera->SetRenderTarget( portraitView );

    g_theRenderer->BeginCamera( uiCamera );
    g_theRenderer->ClearRenderTarget( Rgba::BLACK );
    UpdateActorPortrait();
    m_myActor->RenderPortrait();
    g_theRenderer->EndCamera( uiCamera );

    void* shaderResourceView = portraitView->GetShaderView();
    ImGui::Image( shaderResourceView, ImVec2( windowWidth, windowWidth ), ImVec2( 0.3f, 0.175f ), ImVec2( 0.75f, 0.625f ) );

    // Setup Health Bar

    ImGui::End();
}


