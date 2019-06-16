#pragma once
#include "Game/GameCommon.hpp"


class Actor;
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
    void TakeClosestStairs() const;

    protected:
    Actor* m_myActor = nullptr;


    Map* GetMap() const;
    const StatsManager* GetActorStats() const;

    void SetMoveDir( const Vec2& moveDir ) const;
    void ToggleInventory() const;
    void PickupClosestItem() const;

    void UpdateActorPortrait() const;
};
