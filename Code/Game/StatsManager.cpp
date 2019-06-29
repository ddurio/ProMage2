#include "Game/StatsManager.hpp"

#include "Engine/Math/MathUtils.hpp"

#include "Game/Actor.hpp"


StatsManager::StatsManager( const XMLElement& element ) {
    const XMLElement* childEle = element.FirstChildElement();

    while( childEle != nullptr ) {
        std::string tagName = childEle->Name();

        if( tagName == "Health" ) {
            m_maxHealth = ParseXMLAttribute( *childEle, "max", m_maxHealth );
            m_health = m_maxHealth;

            m_healthRegen = ParseXMLAttribute( *childEle, "regenPerSecond", m_healthRegen );
        } else if( tagName == "MoveSpeed" ) {
            m_moveSpeed = ParseXMLAttribute( *childEle, "value", m_moveSpeed );
        } else if( tagName == "Defense" ) {
            m_defensePower = ParseXMLAttribute( *childEle, "value", m_defensePower );
        }

        childEle = childEle->NextSiblingElement();
    }
}


float StatsManager::GetHealth() const {
    return m_health;
}


float StatsManager::GetMaxHealth() const {
    return m_maxHealth;
}


float StatsManager::GetPercentHealth() const {
    if( m_maxHealth == 0.f ) {
        return 0.f;
    }

    return m_health / m_maxHealth;
}


float StatsManager::GetHealthRegen() const {
    return m_healthRegen;
}


float StatsManager::GetMoveSpeed() const {
    return m_moveSpeed;
}


float StatsManager::GetPickupRadius() const {
    return m_pickupRadius;
}


float StatsManager::GetInteractRadius() const {
    return m_interactRadius;
}


void StatsManager::TakeDamage( float damageToTake ) {
    if( !m_myActor->IsKillable() ) {
        return;
    }

    m_health = ClampFloat( m_health - damageToTake, 0.f, m_maxHealth );

    if( m_health <= 0.f ) {
        m_myActor->Die();
    }
}


void StatsManager::Revive() {
    m_health = m_maxHealth;
    // DFS1FIXME: Could have penalties here
}
