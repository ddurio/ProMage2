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

struct WeaponInfo;


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
    friend class ActorDef;
    friend class ActorController;

    public:
    explicit Actor( Map* theMap, const std::string& actorType, const std::string& controllerType );
    ~Actor();

    static void SetupSpawnActorMGS();
    static bool CompareActorDrawOrder( const Actor* const& actorA, const Actor* const& actorB );

    void Startup();
    void Shutdown();

    void Die() override;
    void TakeDamage( float damageToTake ) override;
    void Revive();

    void Update( float deltaSeconds );
    void Render() const;
    void RenderPortrait() const;

    bool InteractWithActor( Actor* instigator );
    void StartAttack( bool isAttacking = true );
    void Attack();

    std::string GetActorType() const;
    Vec2 GetMoveDir() const;
    ActorController* GetController() const;
    Inventory* GetInventory() const;
    bool IsAttacking() const;
    float GetAttackRange() const;
    float GetAttackDamage() const;
    WeaponInfo GetWeaponInfo() const;

    private:
    const ActorDef* m_actorDef = nullptr;
    Inventory* m_inventory = nullptr;
    ActorController* m_controller = nullptr;

    Vec2 m_moveDir = Vec2::ZERO;

    std::string m_paperDollSprites[NUM_PAPER_DOLL_SLOTS] = { "" };
    Animator* m_animator = nullptr;
    bool m_isAttacking = false;

    GPUMesh* m_portraitMesh = nullptr;

    void UpdateFromController( float deltaSeconds );
    void UpdateHealthBar() const;

    void BuildMesh( const Rgba& tint = Rgba::WHITE ) override;
    void BuildPortraitMesh( const Rgba& ting = Rgba::WHITE );
};
