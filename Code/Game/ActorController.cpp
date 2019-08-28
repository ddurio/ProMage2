#include "Game/ActorController.hpp"

#include "Engine/Math/RNG.hpp"

#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/GameStatePlay.hpp"
#include "Game/Inventory.hpp"
#include "Game/Map.hpp"
#include "Game/Metadata.hpp"
#include "Game/StatsManager.hpp"
#include "Game/Tile.hpp"


ActorController::ActorController( Actor* myActor ) :
    m_myActor( myActor ) {
}


ActorController::~ActorController() {

}


void ActorController::Startup() {

}


void ActorController::Shutdown() {

}


std::string ActorController::GetDeathEvent() const {
    return "actorDeath";
}


bool ActorController::TakeClosestStairs() const {
    const Tile& tile = m_myActor->m_map->GetTileFromWorldCoords( m_myActor->GetPosition() );
    const Metadata* metadata = tile.GetMetadata();

    if( metadata->m_tagData.HasTags( TAG_STAIRS_DOWN ) ) {
        GameStatePlay* state = (GameStatePlay*)g_theGame->GetGameState();
        state->ChangeFloorsDown();

        return true;
    } else if( metadata->m_tagData.HasTags( TAG_STAIRS_UP ) ) {
        GameStatePlay* state = (GameStatePlay*)g_theGame->GetGameState();
        state->ChangeFloorsUp();

        return true;
    }

    return false;
}


bool ActorController::InteractWithActor( Actor* instigator ) {
    UNUSED( instigator );
    return false;
}


// ----- Protected -----
Map* ActorController::GetMap() const {
    return m_myActor->m_map;
}


const StatsManager* ActorController::GetActorStats() const {
    return m_myActor->m_statsManager;
}


Inventory* ActorController::GetActorInventory( const Actor* actor /*= nullptr */ ) const {
    if( actor == nullptr ) {
        actor = m_myActor;
    }

    return actor->m_inventory;
}


Animator* ActorController::GetActorAnimator() const {
    return m_myActor->m_animator;
}


void ActorController::SetMoveDir( const Vec2& moveDir ) const {
    m_myActor->m_moveDir = moveDir;
}


void ActorController::ToggleInventory() const {
    m_myActor->m_inventory->ToggleInventory();
}


// Returns true if the merchant was interacted with
bool ActorController::InteractFromInput() const {
    Strings actions;

    // Look for item in range
    Inventory* mapInventory = m_myActor->m_map->GetMapInventory();
    const StatsManager* actorStats = GetActorStats();
    Item* itemToPickUp = mapInventory->GetClosestItemInRange( m_myActor->GetPosition(), actorStats->GetPickupRadius() );

    if( itemToPickUp != nullptr ) {
        actions.push_back( "item" );
    }


    // Look for tile tags
    const Tile& tile = m_myActor->m_map->GetTileFromWorldCoords( m_myActor->GetPosition() );
    const Tags& tags = tile.GetMetadata()->m_tagData;
    Actor* merchant = nullptr;
    Actor* credits = nullptr;

    if( tags.HasTags( TAG_MERCHANT ) ) {
        merchant = m_myActor->m_map->GetActorInRange( "merchant", m_myActor->GetPosition(), actorStats->GetInteractRadius() );

        if( merchant != nullptr ) {
            actions.push_back( TAG_MERCHANT );
        }
    } else if( tags.HasTags( TAG_CREDITS ) ) {
        credits = m_myActor->m_map->GetActorInRange( "credits", m_myActor->GetPosition(), actorStats->GetInteractRadius() );

        if( credits != nullptr ) {
            actions.push_back( TAG_CREDITS );
        }
    }


    // Pick an action
    int numActions = (int)actions.size();

    if( numActions > 0 ) {
        int actionIndex = g_RNG->GetRandomIntLessThan( numActions );
        std::string& chosenAction = actions[actionIndex];

        if( chosenAction == "item" ) {
            PickupItem( itemToPickUp );
            return false;
        } else if( chosenAction == TAG_MERCHANT ) {
            // Disable for Thesis
            /*
            merchant->InteractWithActor( m_myActor );
            return true;
            */
        } else if( chosenAction == TAG_CREDITS ) {
            credits->InteractWithActor( m_myActor );
            return true;
        }
    }

    return false;
}


void ActorController::PickupItem( Item* itemToPickUp ) const {
    Inventory* mapInventory = m_myActor->m_map->GetMapInventory();

    if( itemToPickUp != nullptr ) {
        mapInventory->RemoveItemFromInventory( itemToPickUp );
        m_myActor->m_inventory->AddItemToInventory( itemToPickUp );
    }
}


void ActorController::UpdateActorPortrait() const {
    m_myActor->BuildPortraitMesh();
}
