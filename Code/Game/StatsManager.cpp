#include "Game/StatsManager.hpp"


StatsManager::StatsManager( const XMLElement& element ) {
    const XMLElement* childEle = element.FirstChildElement();

    while( childEle != nullptr ) {
        std::string tagName = childEle->Name();

        if( tagName == "Health" ) {
            m_maxHealth = ParseXMLAttribute( *childEle, "value", m_maxHealth );
            m_health = m_maxHealth;
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


float StatsManager::GetMoveSpeed() const {
    return m_moveSpeed;
}


float StatsManager::GetPickupRadius() const {
    return m_pickupRadius;
}


void StatsManager::TakeDamage( float damageToTake ) {
    m_health -= damageToTake;
}
