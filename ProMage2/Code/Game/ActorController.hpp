#pragma once
#include "Game/GameCommon.hpp"


class Actor;
class Animator;
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

    virtual std::string GetDeathEvent() const;

    bool TakeClosestStairs() const;
    virtual bool InteractWithActor( Actor* instigator );

    protected:
    Actor* m_myActor = nullptr;


    Map* GetMap() const;
    const StatsManager* GetActorStats() const;
    Inventory* GetActorInventory( const Actor* actor = nullptr ) const;
    Animator* GetActorAnimator() const;

    void SetMoveDir( const Vec2& moveDir ) const;
    void ToggleInventory() const;

    bool InteractFromInput() const;
    void PickupItem( Item* itemToPickup ) const;

    void UpdateActorPortrait() const;
};
