#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Math/Vec2.hpp"

#include "Game/ActorController.hpp"


class EnemyController : public ActorController {
    public:
    EnemyController( Actor* myActor );
    ~EnemyController();

    void Update( float deltaSeconds ) override;
    
    std::string GetDeathEvent() const override;

    private:
    Actor* m_chaseTarget = nullptr;
    bool m_chaseLastKnownPos = false;
    Vec2 m_targetLastKnownPos = Vec2::ZERO;
};
