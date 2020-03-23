#include "Game/EnemyController.hpp"

#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RNG.hpp"

#include "Game/Actor.hpp"
#include "Game/Inventory.hpp"
#include "Game/Item.hpp"
#include "Game/MapGen/Map/Map.hpp"


static bool g_progressionParsed = false;
static std::vector< int > g_mobGearKeyFrames;
static std::vector< IntRange > g_mobGearValues;


static void ParseMobProgression() {
    XmlDocument doc;
    const XMLElement& root = ParseXMLRootElement( DATA_PROGRESSION, doc );

    const XMLElement* mobEle = root.FirstChildElement( "Mobs" );
    const XMLElement* keyEle = mobEle->FirstChildElement( "KeyFrame" );

    while( keyEle != nullptr ) {
        int floorIndex    = ParseXMLAttribute( *keyEle, "floor",    0 );
        IntRange numItems = ParseXMLAttribute( *keyEle, "numItems", IntRange::ONE );

        g_mobGearKeyFrames.push_back( floorIndex );
        g_mobGearValues.push_back( numItems );
        keyEle = keyEle->NextSiblingElement( "KeyFrame" );
    }
};


EnemyController::EnemyController( Actor* myActor ) :
    ActorController( myActor ) {
    if( !g_progressionParsed ) {
        ParseMobProgression();
        g_progressionParsed = true;
    }

    m_myActor->SetFaction( FACTION_ENEMY1 );
    Inventory* inventory = m_myActor->GetInventory();

    // Weapon
    bool getSpear = g_RNG->PercentChance( 0.5f );
    std::string weaponType = getSpear ? "Spear" : "Dagger";
    Item* weapon = inventory->SpawnNewItem( weaponType );
    inventory->EquipItem( weapon );

    // Armor
    // Get number of items based on Progression.xml info
    Map* theMap = GetMap();
    int floorIndex = theMap->GetCurrentFloor();
    int numKeys = (int)g_mobGearKeyFrames.size();
    int keyIndex = 1;

    while( (float)floorIndex > g_mobGearKeyFrames[keyIndex] ) {
        if( keyIndex == numKeys - 1 ) {
            break;
        }

        keyIndex++;
    }

    int floorMin = g_mobGearKeyFrames[keyIndex - 1];
    int floorMax = g_mobGearKeyFrames[keyIndex];
    IntRange rangeMin = g_mobGearValues[keyIndex - 1];
    IntRange rangeMax = g_mobGearValues[keyIndex];

    float scaledFloor = RangeMap( (float)floorIndex, (float)floorMin, (float)floorMax, 0.f, 1.f );
    float scaledMin = RangeMap( scaledFloor, 0.f, 1.f, (float)rangeMin.min, (float)rangeMax.min );
    float scaledMax = RangeMap( scaledFloor, 0.f, 1.f, (float)rangeMin.max, (float)rangeMax.max );

    int scaledMinInt = RoundToInt( scaledMin );
    scaledMinInt = Max( scaledMinInt, 0 );

    int scaledMaxInt = RoundToInt( scaledMax );
    scaledMaxInt = Max( scaledMaxInt, 0 );

    int numNewItems = g_RNG->GetRandomIntInRange( scaledMinInt, scaledMaxInt );

    // Spawn new items
    for( int itemIndex = 0; itemIndex < numNewItems; itemIndex++ ) {
        Item* newItem = theMap->SpawnLootDrop( inventory );

        if( newItem == nullptr ) {
            break;
        }

        // Equip it if the new item is better
        ItemSlot slot = newItem->GetItemSlot();
        Item* equippedItem = inventory->GetItemInSlot( slot );

        if( equippedItem == nullptr ) {
            inventory->EquipItem( newItem );
        } else {
            float equippedValue = (slot == ITEM_SLOT_WEAPON) ? equippedItem->GetAttackDamage() : equippedItem->GetDefense();
            float newValue =      (slot == ITEM_SLOT_WEAPON) ? newItem->GetAttackDamage()      : newItem->GetDefense();

            if( newValue > equippedValue ) {
                inventory->EquipItem( newItem );
            }
        }
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
