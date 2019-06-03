#include "Game/ActorController.hpp"

#include "Game/Actor.hpp"


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
