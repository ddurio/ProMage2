#include "Game/EnemyController.hpp"

#include "Engine/Math/RNG.hpp"

#include "Game/Actor.hpp"
#include "Game/Inventory.hpp"
#include "Game/Map.hpp"


EnemyController::EnemyController( Actor* myActor ) :
    ActorController( myActor ) {
    Inventory* inventory = m_myActor->GetInventory();

    // Weapon
    bool getSpear = g_RNG->PercentChance( 0.5f );
    std::string weaponType = getSpear ? "Spear" : "Dagger";
    Item* weapon = inventory->SpawnNewItem( weaponType );
    inventory->EquipItem( weapon );

    // Armor
    Map* theMap = GetMap();
    int numNewItems = g_RNG->GetRandomIntInRange( 1, 5 );

    for( int itemIndex = 0; itemIndex < numNewItems; itemIndex++ ) {
        Item* newItem = theMap->SpawnLootDrop( inventory );

        if( newItem == nullptr ) {
            break;
        }

        inventory->EquipItem( newItem );
    }
}


EnemyController::~EnemyController() {

}


void EnemyController::Update( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    Map* theMap = GetMap();

    if( m_chaseTarget == nullptr || !m_chaseTarget->IsAlive() ) {
        // Look for target
        m_chaseTarget = theMap->GetActorInSight( m_myActor );

        if( m_chaseTarget == nullptr ) {
            m_myActor->StartAttack( false );
            //m_myActor->SetAttackTarget( nullptr );
            return; // DFS1FIXME: Just wait for now
        }
    }

    bool hasLoS = theMap->HasLineOfSight( m_myActor, m_chaseTarget );
    float attackRange = m_myActor->GetAttackRange();

    Vec2 targetPos = m_chaseTarget->GetPosition();
    Vec2 myPos     = m_myActor->GetPosition();
    Vec2 targetDisplacement = (targetPos - myPos);

    if( hasLoS ) { // Have LoS, Chase actor
        m_chaseLastKnownPos = false;
        m_targetLastKnownPos = targetPos;

        if( targetDisplacement.GetLengthSquared() < attackRange * attackRange ) {
            m_myActor->StartAttack();
            //m_myActor->SetAttackTarget( m_chaseTarget );
            SetMoveDir( Vec2::ZERO ); // Stop while attacking
        } else {
            m_myActor->StartAttack( false );
            //m_myActor->SetAttackTarget( nullptr );
            SetMoveDir( targetDisplacement.GetNormalized() );
        }
    } else { // No LoS, Chase last known position
        m_chaseLastKnownPos = true;
        targetDisplacement = m_targetLastKnownPos - myPos;
        SetMoveDir( targetDisplacement.GetNormalized() );

        if( targetDisplacement.GetLengthSquared() < 0.01f ) { // Out of sight, out of mind
            m_chaseTarget = nullptr;
            m_chaseLastKnownPos = false;
            SetMoveDir( Vec2::ZERO );
        }
    }
}


std::string EnemyController::GetDeathEvent() const {
    return EVENT_DEATH_ENEMY;
}
