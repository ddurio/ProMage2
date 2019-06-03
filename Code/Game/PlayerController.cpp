#include "Game/PlayerController.hpp"

#include "Game/Game.hpp"
#include "Game/GameInput.hpp"


PlayerController::PlayerController( Actor* myActor, int playerID ) :
    ActorController( myActor ),
    m_playerIndex( playerID ) {
    m_gameInput = g_theGame->GetGameInput();
}


PlayerController::~PlayerController() {

}


void PlayerController::Update( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    Vec2 moveDir = m_gameInput->GetMovementDirection();
    SetMoveDir( moveDir );
}
