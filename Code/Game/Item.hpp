#pragma once
#include "Game/GameCommon.hpp"

#include "Game/Entity.hpp"
#include "Game/ItemDef.hpp"


class Tile;


class Item : public Entity {
    friend class ItemDef;

    public:
    explicit Item( Map* theMap, std::string itemType );
    explicit Item( Map* theMap, const ItemDef* itemDef );

    void Startup();
    void Shutdown();

    void Die();

    void Update( float deltaSeconds );
    void Render() const;

    void OnCollisionEntity( Entity* collidingEntity );
    void OnCollisionTile( Tile* collidingTile );

    ItemSlot GetItemSlot() const;

    void SetWorldPosition( const Vec2& worldPosition );

    private:
    const ItemDef* m_itemDef = nullptr;

    void BuildMesh( const Rgba& tint = Rgba::WHITE ) override;
};
