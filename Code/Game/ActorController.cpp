#include "Game/ActorController.hpp"

#include "Game/Actor.hpp"
#include "Game/Inventory.hpp"


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


void ActorController::UpdateActorPortrait() const {
    m_myActor->BuildPortraitMesh();
}
