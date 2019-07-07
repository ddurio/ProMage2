#include "Game/PlayerController.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/TextureView2D.hpp"

#include "Game/Actor.hpp"
#include "Game/Animator.hpp"
#include "Game/Game.hpp"
#include "Game/GameInput.hpp"
#include "Game/GameState.hpp"
#include "Game/Inventory.hpp"
#include "Game/Map.hpp"
#include "Game/StatsManager.hpp"


PlayerController::PlayerController( Actor* myActor ) :
    ActorController( myActor ) {
    m_myActor->SetFaction( FACTION_PLAYER0 );
    m_gameInput = g_theGame->GetGameInput();

    g_theRenderer->GetOrCreateRenderTarget( m_portraitViewName );

    // Give starter weapon
    Inventory* inventory = m_myActor->GetInventory();
    Item* dagger = inventory->SpawnNewItem( "StarterDagger" );
    inventory->EquipItem( dagger );

    // Mark actor as player
    Map* theMap = GetMap();
    theMap->SetPlayer( myActor );
}


PlayerController::~PlayerController() {
    Map* theMap = GetMap();
    theMap->ClearPlayer( m_myActor );
}


void PlayerController::Update( float deltaSeconds ) {
    bool inventoryWasOpen = m_inventoryOpen;
    bool tradeWasOpen = m_tradeOpen;


    // Movement
    Vec2 moveDir = m_gameInput->GetMovementDirection();
    SetMoveDir( moveDir );


    // Health Regen
    const StatsManager* statsManager = GetActorStats();
    float regenPerSecond = statsManager->GetHealthRegen();
    m_myActor->TakeDamage( -regenPerSecond * deltaSeconds ); // Negative damage == healing


    // Inventory
    if( !m_tradeOpen && m_gameInput->WasInventoryToggled() ) {
        m_inventoryOpen = !m_inventoryOpen;
        ToggleInventory();
    }


    // Pickup Item or Merchant (can't do either while already trading)
    if( !m_inventoryOpen && !m_tradeOpen && m_gameInput->ShouldInteract() ) {
        m_tradeOpen = InteractFromInput();
    }


    // Exit menus
    if( m_gameInput->ShouldExitMenu() ) {
        if( m_inventoryOpen ) {
            m_inventoryOpen = false;
            ToggleInventory();
        } else if( m_tradeOpen ) {
            // Merchant will close the window
            m_tradeOpen = false;
        }
    }

    
    // Attack
    if( m_gameInput->ShouldAttack() ) {
        m_myActor->StartAttack();
    } else if( m_myActor->IsAttacking() ) {
        Animator* myAnimator = GetActorAnimator();
        std::string currentAnim = myAnimator->GetCurrentAnimName();

        if( myAnimator->AnimHasFinished() ||
            (currentAnim != ANIM_PAPER_DOLL_SHOOT &&
             currentAnim != ANIM_PAPER_DOLL_SLASH &&
             currentAnim != ANIM_PAPER_DOLL_SPELL &&
             currentAnim != ANIM_PAPER_DOLL_THRUST ) ) {

            m_myActor->StartAttack( false );
        }
    }


    // Pause / Resume game
    Clock* stateClock = g_theGame->GetGameState()->GetStateClock();

    if( (m_inventoryOpen && !inventoryWasOpen) ||
        (m_tradeOpen && !tradeWasOpen) ) {
        stateClock->Pause();
    } else if( (!m_inventoryOpen && inventoryWasOpen) ||
        (!m_tradeOpen && tradeWasOpen) ) {
        stateClock->Unpause();
    }


    UpdateHUD();
}


std::string PlayerController::GetDeathEvent() const {
    return EVENT_DEATH_PLAYER;
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


