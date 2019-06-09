#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/XMLUtils.hpp"


class StatsManager {
    public:
    StatsManager() {};
    StatsManager( const XMLElement& element );
    ~StatsManager() {};

    float GetHealth() const;
    float GetMaxHealth() const;
    float GetPercentHealth() const;

    float GetMoveSpeed() const;

    void TakeDamage( float damageToTake );

    private:
    float m_maxHealth = 100.f;
    float m_health = m_maxHealth;

    float m_moveSpeed = 3.f;
    float m_defensePower = 0.f;
};
