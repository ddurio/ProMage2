#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/Tags.hpp"

#include "Game/Actor.hpp"
#include "Game/ItemDef.hpp"


class Item;
class Map;

class Inventory {
    public:
    explicit Inventory( Map* map, bool renderEquippedItems = true, bool renderUnequippedItems = false );
    ~Inventory();

    void Update( float deltaSeconds );
    void UpdateItemPositions( const Vec2&  worldPosition );
    void UpdatePaperDoll( std::string (&dollSprites)[NUM_PAPER_DOLL_SLOTS] ) const;

    void Render() const;

    Item* SpawnNewItem( std::string itemType, const Vec2& worldPosition = Vec2::ZERO );

    void AddItemToInventory( Item* itemToAdd );
    void RemoveItemFromInventory( Item* itemToRemove );

    void EquipItem( Item* itemToEquip, bool removeFromInventory = true );
    void UnequipItem( Item* itemToUnequip );

    void SetRenderPreferences( bool renderEquippedItems = true, bool renderUnequippedItems = false );
    void AddItemSets( const Strings& validSetsVec );
    void AddItemSets( const std::string& validSetCSV );

    Item* GetItemAtPosition( const Vec2& worldPosition ) const;
    Item* GetItemInSlot( int unequippedSlotIndex ) const;
    Item* GetItemInSlot( ItemSlot equippedItemSlot ) const;

    private:
    Map* m_map = nullptr;
    std::vector<Item*> m_unequippedItems;
    Item* m_equippedItems[NUM_ITEM_SLOTS] = { nullptr };

    bool m_renderEquippedItems = true;
    bool m_renderUnequippedItems = false;
    //std::vector< std::string > m_validItemSets;
    //std::vector< std::string > m_requiredItemSets;
    Tags m_itemSets;


    bool IsItemEquipable( const Item* itemToEquip ) const;
};
