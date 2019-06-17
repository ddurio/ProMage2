#pragma once
#include "Game/GameCommon.hpp"


class Actor;
class Inventory;
class Item;
class Map;
class StatsManager;


class ActorController {
    public:
    ActorController( Actor* myActor );
    virtual ~ActorController();

    void Startup();
    void Shutdown();

    virtual void Update( float deltaSeconds ) = 0;
    bool TakeClosestStairs() const;

    virtual bool InteractWithActor( Actor* instigator );

    protected:
    Actor* m_myActor = nullptr;


    Map* GetMap() const;
    const StatsManager* GetActorStats() const;
    Inventory* GetActorInventory( const Actor* actor = nullptr ) const;

    void SetMoveDir( const Vec2& moveDir ) const;
    void ToggleInventory() const;

    void InteractFromInput() const;

    void PickupItem( Item* itemToPickup ) const;

    void UpdateActorPortrait() const;
};
