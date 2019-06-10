#include "Game/PlayerController.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView2D.hpp"

#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/GameInput.hpp"
#include "Game/StatsManager.hpp"


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
    if( m_gameInput->WasInvtoryToggled() ) {
        ToggleInventory();
    }

    // Pickup Item
    if( m_gameInput->ShouldPickupItem() ) {
        PickupClosestItem();
    }

    UpdateHUD();
}

void PlayerController::UpdateHUD() {
    // Setup Window
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

    AABB2 clientBounds = g_theWindow->GetClientBounds();
    Vec2 clientDimensions = clientBounds.GetDimensions();

    float portraitWidth = 0.1f * clientDimensions.x;

    ImGuiStyle& portraitStyle = ImGui::GetStyle();
    Vec2 portraitSize = Vec2( portraitWidth, -portraitWidth ) + Vec2( portraitStyle.WindowPadding.x, portraitStyle.WindowPadding.y );

    AABB2 portrait = clientBounds.GetBoxWithin( portraitSize, Vec2::ZERO );
    Vec2 portraitOrigin = Vec2( portrait.mins.x, portrait.maxs.y - 10.f );

    ImGui::SetNextWindowPos( ImVec2( portraitOrigin.x, portraitOrigin.y ), ImGuiCond_Always );
    ImGui::SetNextWindowSize( ImVec2( portraitWidth, portraitWidth ), ImGuiCond_Always );
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
    ImGui::Image( shaderResourceView, ImVec2( portraitWidth, portraitWidth ), ImVec2( 0.3f, 0.175f ), ImVec2( 0.75f, 0.625f ) );
    ImGui::End();


    // Setup Health Bar
    Vec2 healthSize = Vec2( 2.f * portraitWidth, 0.1f * portraitWidth );
    Vec2 healthOrigin = Vec2( portraitOrigin.x + portraitWidth, portraitOrigin.y + (0.8f * portraitWidth) );

    ImGui::SetNextWindowPos( ImVec2( healthOrigin.x, healthOrigin.y ), ImGuiCond_Always );
    ImGui::SetNextWindowSize( ImVec2( healthSize.x, healthSize.y ), ImGuiCond_Always );
    ImGui::Begin( "HealthBar", nullptr, windowFlags );

    const StatsManager* actorStats = GetActorStats();

    float health = actorStats->GetHealth();
    float maxHealth = actorStats->GetMaxHealth();
    float healthPercent = actorStats->GetPercentHealth();

    std::string healthText = Stringf( "%d/%d", (int)health, (int)maxHealth );

    ImGuiStyle& healthStyle = ImGui::GetStyle();
    ImVec4 origColor = healthStyle.Colors[ImGuiCol_PlotHistogram];
    healthStyle.Colors[ImGuiCol_PlotHistogram] = ImVec4( Rgba::RED.r, Rgba::RED.g, Rgba::RED.b, Rgba::RED.a );

    ImGui::ProgressBar( healthPercent, ImVec2( -1, 0 ), healthText.c_str() );

    healthStyle.Colors[ImGuiCol_PlotHistogram] = origColor;
    ImGui::End();
}


