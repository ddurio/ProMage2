#pragma once
#include "Game/GameCommon.hpp"

#include "Game/Entity.hpp"
#include "Game/ActorDef.hpp"


class ActorController;
class Animator;
class Inventory;
class StatsManager;
class Tile;
class Timer;


// ORDER MUST MATCH PaperDoll.hlsl FOR ALL
enum PaperDollSlot {
    // ORDER MUST MATCH ITEM_SLOT ORDER
    PAPER_DOLL_HELM,
    PAPER_DOLL_CHEST,
    PAPER_DOLL_SHOULDER,
    PAPER_DOLL_LEGS,
    PAPER_DOLL_FEET,
    PAPER_DOLL_WEAPON,

    // These are unique to PaperDoll, must be at end
    PAPER_DOLL_BODY,
    PAPER_DOLL_EARS,
    PAPER_DOLL_HAIR,

    NUM_PAPER_DOLL_SLOTS
};



class Actor : public Entity {
    friend class Definition<Actor>;
    friend class ActorController;

    public:
    explicit Actor( Map* theMap, std::string actorType, int playerID = -1 );
    //explicit Actor( Map* theMap, const Definition<Actor>* actorDef, int playerID = -1 );
    ~Actor();

    void Startup();
    void Shutdown();

    void Die();

    void Update( float deltaSeconds );
    void Render() const;
    void RenderPortrait() const;

    void OnCollisionEntity( Entity* collidingEntity );
    void OnCollisionTile( Tile* collidingTile );

    Vec2 GetMoveDir() const;

    private:
    const Definition<Actor>* m_actorDef = nullptr;
    Inventory* m_inventory = nullptr;
    ActorController* m_controller = nullptr;
    StatsManager* m_statsManager = nullptr;

    Vec2 m_moveDir = Vec2::ZERO;

    std::string m_paperDollSprites[NUM_PAPER_DOLL_SLOTS] = { "" };
    Animator* m_animator = nullptr;

    GPUMesh* m_portraitMesh = nullptr;

    void UpdateFromController( float deltaSeconds );

    void BuildMesh( const Rgba& tint = Rgba::WHITE ) override;
    void BuildPortraitMesh( const Rgba& ting = Rgba::WHITE );
};
