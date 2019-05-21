#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"

#include "map"


class Item;
class SpriteSheet;
class Texture;

enum ItemSlot {
    ITEM_SLOT_NONE = -1,
    ITEM_SLOT_MAIN_HAND,
    ITEM_SLOT_OFF_HAND,

    NUM_ITEM_SLOTS
};

class ItemDef {
    public:
    explicit ItemDef( const XMLElement& element );

    static void InitializeItemDefs();
    static void DestroyItemDefs();
    static const ItemDef* GetItemDef( std::string itemType );

    void Define( Item& item ) const;

    const std::string& GetItemType() const;
    const ItemSlot GetItemSlot() const;
    std::string GetTexturePath() const;
    int GetPPU() const;
    const AABB2& GetUVs() const;

    private:
    static std::map<std::string, ItemDef*> s_itemDefs;

    std::string m_itemType = "";
    ItemSlot m_itemSlot = ITEM_SLOT_NONE;
    SpriteSheet* m_spriteSheet = nullptr;
    IntVec2 m_spriteCoords = IntVec2::ZERO;
    AABB2 m_spriteUVs = AABB2::ZEROTOONE;
    int m_imagePPU = 1;
};
