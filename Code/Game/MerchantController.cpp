#include "Game/MerchantController.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/WindowContext.hpp"

#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/GameInput.hpp"
#include "Game/Inventory.hpp"


MerchantController::MerchantController( Actor* myActor ) :
    ActorController( myActor ) {
    m_gameInput = g_theGame->GetGameInput();
}


MerchantController::~MerchantController() {

}


void MerchantController::Update( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    if( m_interactingWith != nullptr ) {
        if( m_gameInput->ShouldExitMenu() ) {
            m_interactingWith = nullptr;
            return;
        }

        UpdateTradeUI();
    }
}


bool MerchantController::InteractWithActor( Actor* instigator ) {
    m_interactingWith = instigator;
    return true;
}


// ----- Private -----
void MerchantController::UpdateTradeUI() const {
    CreateUIWindow();

    // Player(?) inventory on left
    Inventory* otherInventory = GetActorInventory( m_interactingWith );
    otherInventory->AddUIContent();
    //ImGui::Text( "%x", otherInventory );
    ImGui::NextColumn();

    // Merchant inventory on right
    Inventory* myInventory = GetActorInventory();
    myInventory->AddUIContent();
    //ImGui::Text( "%x", myInventory );

    ImGui::End();
}


void MerchantController::CreateUIWindow() const {
    // Setup Window
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    AABB2 clientBounds = g_theWindow->GetClientBounds();
    Vec2 clientDimensions = clientBounds.GetDimensions();
    clientDimensions.y = -clientDimensions.y;

    Vec2 clientCenter = clientBounds.GetCenter();

    Vec2 windowSize = 0.9f * clientDimensions;
    Vec2 windowOrigin = clientCenter - (0.5f * windowSize);

    ImGui::SetNextWindowPos( ImVec2( windowOrigin.x, windowOrigin.y ), ImGuiCond_Always );
    ImGui::SetNextWindowSize( ImVec2( windowSize.x, windowSize.y ), ImGuiCond_Always );
    ImGui::Begin( "Trade", nullptr, windowFlags );

    // Setup Columns
    ImGui::Columns( 8, nullptr, false );

    float halfWidth = 0.5f * windowSize.x;
    float equipmentSize = 0.25f * halfWidth;
    float equipmentColSize = 0.333f * equipmentSize;

    // Instigator
    ImGui::SetColumnWidth( 0, 0.75f * halfWidth );
    ImGui::SetColumnWidth( 1, equipmentColSize );
    ImGui::SetColumnWidth( 2, equipmentColSize );
    ImGui::SetColumnWidth( 3, equipmentColSize );

    // Merchant
    ImGui::SetColumnWidth( 4, 0.75f * halfWidth );
    ImGui::SetColumnWidth( 5, equipmentColSize );
    ImGui::SetColumnWidth( 6, equipmentColSize );
    ImGui::SetColumnWidth( 7, equipmentColSize );
}


