#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/XMLUtils.hpp"

#include "Game/Definition.hpp"


class Actor;


class StatsManager {
    friend class Definition<Actor>;

    public:
    StatsManager() {};
    explicit StatsManager( const XMLElement& element );
    ~StatsManager() {};

    float GetHealth() const;
    float GetMaxHealth() const;
    float GetPercentHealth() const;
    float GetHealthRegen() const;

    float GetMoveSpeed() const;
    float GetPickupRadius() const;
    float GetInteractRadius() const;

    void TakeDamage( float damageToTake );
    void Revive();

    private:
    Actor* m_myActor = nullptr;

    float m_maxHealth = 100.f;
    float m_health = m_maxHealth;
    float m_healthRegen = 0.f;

    float m_moveSpeed = 3.f;
    float m_defensePower = 0.f;

    float m_pickupRadius = 1.f;
    float m_interactRadius = 2.f;
};
