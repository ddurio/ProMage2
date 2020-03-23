#pragma once
#include "Game/GameCommon.hpp"

#include "Game/ActorController.hpp"


class GameInput;

class PlayerController : public ActorController {
    public:
    PlayerController( Actor* myActor );
    ~PlayerController();

    void Update( float deltaSeconds ) override;

    std::string GetDeathEvent() const override;

    private:
    GameInput* m_gameInput = nullptr;
    std::string m_portraitViewName = "playerController/portraitView";

    bool m_inventoryOpen = false;
    bool m_tradeOpen = false;


    void UpdateHUD();
};
