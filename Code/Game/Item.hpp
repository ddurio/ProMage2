#pragma once
#include "Game/GameCommon.hpp"

#include "Game/Entity.hpp"
#include "Game/ItemDef.hpp"


class Actor;
class SpriteDef;
class Tags;
class Tile;


enum AttackType {
    ATTACK_NONE = -1,
    ATTACK_MELEE,
    ATTACK_RANGED
};

struct WeaponInfo {
    AttackType type = ATTACK_NONE;
    float range = -1.f;
    float damage = 0.f;
    float coneDotProduct = -1.f;
};


class Item : public Entity {
    friend class ItemDef;

    public:
    explicit Item( Map* theMap, std::string itemType );
    //explicit Item( Map* theMap, const Definition<Item>* itemDef );

    void Startup();
    void Shutdown();

    void Die();

    void Update( float deltaSeconds );
    void Render() const;

    void OnCollisionEntity( Entity* collidingEntity );
    void OnCollisionTile( Tile* collidingTile );

    const SpriteDef GetPortrait() const;
    ItemSlot GetItemSlot() const;
    std::vector< Tags > GetItemSets() const;
    std::string GetSprites() const;
    std::string GetSpriteTexture() const;
    std::string GetItemType() const;
    int GetValue() const;
    std::string GetAttackAnim() const;
    float GetAttackRange() const;
    float GetAttackDamage() const;
    float GetAttackConeWidth() const;
    float GetAttackConeDotProduct() const;
    WeaponInfo GetWeaponInfo() const;
    bool IsConsumable() const;

    void SetWorldPosition( const Vec2& worldPosition );
    void Consume( Actor* consumer ) const;

    private:
    const Definition<Item>* m_itemDef = nullptr;

    void BuildMesh( const Rgba& tint = Rgba::WHITE ) override;
};
