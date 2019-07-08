#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/Tags.hpp"

#include "Game/Actor.hpp"
#include "Game/ItemDef.hpp"


class Item;
class Map;

enum ItemSlot : int;

struct ImVec2;
struct ItemTilePayload;


class Inventory {
    public:
    explicit Inventory( Actor* owner, Map*& theMap, bool renderEquippedItems = true, bool renderUnequippedItems = false );
    ~Inventory();

    void Update( float deltaSeconds );
    void UpdateItemPositions( const Vec2&  worldPosition );
    void UpdatePaperDoll( std::string (&dollSprites)[NUM_PAPER_DOLL_SLOTS] ) const;

    void Render() const;

    Item* SpawnNewItem( std::string itemType, const Vec2& worldPosition = Vec2::ZERO, RNG* customRNG = nullptr );

    void TransferMoney( int moneyToTransfer );
    void AddItemToInventory( Item* itemToAdd, int indexToAdd = -1 );
    void RemoveItemFromInventory( Item* itemToRemove );
    void DropItem( Item* itemToDrop );
    void DropItem( int itemIndexToDrop );

    void EquipItem( Item* itemToEquip, bool removeFromInventory = true, int currentItemDestinationIndex = -1 );
    void UnequipItem( Item* itemToUnequip );

    void SetRenderPreferences( bool renderEquippedItems = true, bool renderUnequippedItems = false );
    void SetInventorySize( int numItemSlots );
    void ToggleInventory();

    void AddItemSets( const Strings& validSetsVec );
    void AddItemSets( const std::string& validSetCSV );
    bool HasAllItemSets( const Tags& itemSetTags ) const;
    bool HasOneItemSet( const Tags& itemSetTags ) const;

    Item* GetItemAtPosition( const Vec2& worldPosition ) const;
    Item* GetClosestItemInRange( const Vec2& worldPosition, float pickupRadius ) const;

    Item* GetItemInSlot( int unequippedSlotIndex ) const;
    Item* GetItemInSlot( ItemSlot equippedItemSlot ) const;

    int GetItemIndex( Item* itemToFind, bool& outIsEquipped ) const;
    int GetMoney() const;
    float GetDefense() const;

    void AddUIContent();

    private:
    Actor* m_owner = nullptr;
    Map*& m_map;

    int m_numItemSlots = 50;
    std::vector<Item*> m_unequippedItems;
    Item* m_equippedItems[NUM_ITEM_SLOTS] = { nullptr };
    int m_money = 0;

    bool m_renderEquippedItems = true;
    bool m_renderUnequippedItems = false;
    bool m_isOpen = false;

    Tags m_itemSets;


    void UpdateUI();
    void CreateUIWindow();

    void AddUnequippedItemTile( int itemIndex );
    void AddEquippedItemTile( ItemSlot itemSlot, const ImVec2& tileSize );
    void CreateItemTile( int itemIndex, bool isEquipped, const ImVec2& tileSize, const std::string& emptyTileName = ANIM_INVENTORY_EMPTY );
    void CreateItemTooltip( Item* item ) const;
    void CreateMoneyTile( const ImVec2& tileSize );
    void SetupDragAndDrop( Item* item, int itemIndex, bool isEquipped, void* shaderResourceView, const ImVec2& tileSize, const AABB2& uvs );

    static void HandleDragAndDrop( const ItemTilePayload& sourcePayload, const ItemTilePayload& targetPayload );
    void SwapItems( const ItemTilePayload& sourcePayload, const ItemTilePayload& targetPayload );
    static void TradeItems( const ItemTilePayload& sourcePayload, const ItemTilePayload& targetPayload );
    static void TradeMoney( const Item* sourceItem, Inventory* sourceInv, const Item* targetItem, Inventory* targetInv );

    bool IsItemEquipable( const Item* itemToEquip ) const;
    static bool IsTradeValid( const Item* sourceItem, const Inventory* sourceInv, const Item* targetItem, const Inventory* targetInv );
};
