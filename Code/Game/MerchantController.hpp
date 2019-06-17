#pragma once
#include "Game/GameCommon.hpp"

#include "Game/ActorController.hpp"


class GameInput;


class MerchantController : public ActorController {
    public:
    MerchantController( Actor* myActor );
    ~MerchantController();

    void Update( float deltaSeconds ) override;

    bool InteractWithActor( Actor* instigator ) override;

    private:
    GameInput* m_gameInput = nullptr;

    Actor* m_interactingWith = nullptr;


    void UpdateTradeUI() const;
    void CreateUIWindow() const;
};
