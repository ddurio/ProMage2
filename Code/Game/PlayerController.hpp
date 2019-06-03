#pragma once
#include "Game/GameCommon.hpp"

#include "Game/ActorController.hpp"


class GameInput;

class PlayerController : public ActorController {
    public:
    PlayerController( Actor* myActor, int playerID );
    ~PlayerController();

    void Update( float deltaSeconds ) override;

    private:
    GameInput* m_gameInput = nullptr;
    int m_playerIndex = -1;
};
