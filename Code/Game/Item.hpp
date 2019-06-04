#pragma once
#include "Game/GameCommon.hpp"

#include "Game/Entity.hpp"
#include "Game/ItemDef.hpp"


class SpriteDef;
class Tags;
class Tile;


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

    ItemSlot GetItemSlot() const;
    std::vector< Tags > GetItemSets() const;
    std::string GetSprites() const;

    void SetWorldPosition( const Vec2& worldPosition );

    private:
    const Definition<Item>* m_itemDef = nullptr;

    SpriteDef* m_portraitSprite = nullptr;

    void BuildPortrait();
    void BuildMesh( const Rgba& tint = Rgba::WHITE ) override;
};
