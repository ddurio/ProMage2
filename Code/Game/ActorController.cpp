#include "Game/ActorController.hpp"

#include "Game/Actor.hpp"
#include "Game/Inventory.hpp"
#include "Game/Map.hpp"
#include "Game/StatsManager.hpp"


ActorController::ActorController( Actor* myActor ) :
    m_myActor( myActor ) {
}


ActorController::~ActorController() {

}


void ActorController::Startup() {

}


void ActorController::Shutdown() {

}


void ActorController::SetMoveDir( const Vec2& moveDir ) const {
    m_myActor->m_moveDir = moveDir;
}


void ActorController::ToggleInventory() const {
    m_myActor->m_inventory->ToggleInventory();
}


const StatsManager* ActorController::GetActorStats() const {
    return m_myActor->m_statsManager;
}


void ActorController::PickupClosestItem() const {
    Inventory* mapInventory = m_myActor->m_map->GetMapInventory();
    const StatsManager* actorStats = GetActorStats();

    Item* itemToPickUp = mapInventory->GetClosestItemInRange( m_myActor->GetPosition(), actorStats->GetPickupRadius() );

    if( itemToPickUp != nullptr ) {
        mapInventory->RemoveItemFromInventory( itemToPickUp );
        m_myActor->m_inventory->AddItemToInventory( itemToPickUp );
    }
}


void ActorController::UpdateActorPortrait() const {
    m_myActor->BuildPortraitMesh();
}
