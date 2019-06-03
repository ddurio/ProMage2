#include "Game/Inventory.hpp"

#include "Game/Item.hpp"
#include "Game/Map.hpp"


Inventory::Inventory( Map* map, bool renderEquippedItems /*= false*/, bool renderUnequippedItems /*= false*/ ) :
    m_map(map) {
    SetRenderPreferences( renderEquippedItems, renderUnequippedItems );
}


Inventory::~Inventory() {
    EngineCommon::ClearVector( m_unequippedItems );

    for( int itemIndex = 0; itemIndex < NUM_ITEM_SLOTS; itemIndex++ ) {
        CLEAR_POINTER( m_equippedItems[itemIndex] );
    }
}


void Inventory::Update( float deltaSeconds ) {
    // Unequipped Items
    int numUnequipped = (int)m_unequippedItems.size();
    for( int itemIndex = 0; itemIndex < numUnequipped; itemIndex++ ) {
        Item* item = m_unequippedItems[itemIndex];

        if( item != nullptr ) {
            item->Update( deltaSeconds );
        }
    }

    // Equipped Items
    for( int itemIndex = 0; itemIndex < NUM_ITEM_SLOTS; itemIndex++ ) {
        Item* item = m_equippedItems[itemIndex];

        if( item != nullptr ) {
            item->Update( deltaSeconds );
        }
    }
}


void Inventory::UpdateItemPositions( const Vec2& worldPosition ) {
    // Unequipped Items
    int numUnequipped = (int)m_unequippedItems.size();
    for( int itemIndex = 0; itemIndex < numUnequipped; itemIndex++ ) {
        Item* item = m_unequippedItems[itemIndex];

        if( item != nullptr ) {
            item->SetWorldPosition( worldPosition );
        }
    }

    // Equipped Items
    for( int itemIndex = 0; itemIndex < NUM_ITEM_SLOTS; itemIndex++ ) {
        Item* item = m_equippedItems[itemIndex];

        if( item != nullptr ) {
            item->SetWorldPosition( worldPosition );
        }
    }
}


void Inventory::UpdatePaperDoll( std::string( &dollSprites )[NUM_PAPER_DOLL_SLOTS] ) const {
    for( int slotIndex = 0; slotIndex < NUM_ITEM_SLOTS; slotIndex++ ) {
        const Item* item = m_equippedItems[slotIndex];

        if( item != nullptr ) {
            dollSprites[slotIndex] = item->GetSprites();
        } else {
            dollSprites[slotIndex] = "";
        }
    }
}


void Inventory::Render() const {
    if( m_renderUnequippedItems ) {
        int numUnequipped = (int)m_unequippedItems.size();
        for( int itemIndex = 0; itemIndex < numUnequipped; itemIndex++ ) {
            Item* item = m_unequippedItems[itemIndex];

            if( item != nullptr ) {
                item->Render();
            }
        }
    }

    if( m_renderEquippedItems ) {
        for( int itemIndex = 0; itemIndex < NUM_ITEM_SLOTS; itemIndex++ ) {
            Item* item = m_equippedItems[itemIndex];

            if( item != nullptr ) {
                item->Render();
            }
        }
    }
}


Item* Inventory::SpawnNewItem( std::string itemType, const Vec2& worldPosition /*= Vec3::ZERO */ ) {
    Item* newItem = new Item( m_map, itemType );
    newItem->Startup();
    newItem->SetWorldPosition( worldPosition );

    AddItemToInventory( newItem );
    return newItem;
}


void Inventory::AddItemToInventory( Item* itemToAdd ) {
    // DFS1FIXME: Check for empty equipped slot of the same type and auto equip instead

    int numItems = (int)m_unequippedItems.size();

    for( int itemIndex = 0; itemIndex < numItems; itemIndex++ ) {
        Item* item = m_unequippedItems[itemIndex];

        if( item == nullptr ) {
            m_unequippedItems[itemIndex] = itemToAdd;
            return;
        }
    }

    // No nullptr slots found, pushing to end
    m_unequippedItems.push_back( itemToAdd );
}


void Inventory::RemoveItemFromInventory( Item* itemToRemove ) {
    int numItems = (int)m_unequippedItems.size();

    for( int itemIndex = 0; itemIndex < numItems; itemIndex++ ) {
        Item* item = m_unequippedItems[itemIndex];

        if( item == itemToRemove ) {
            m_unequippedItems[itemIndex] = nullptr;
            return;
        }
    }
}


void Inventory::EquipItem( Item* itemToEquip, bool removeFromInventory /*= true*/ ) {
    ItemSlot slot = itemToEquip->GetItemSlot();

    // This item is not equipable
    if( !IsItemEquipable(itemToEquip) ) {
        return;
    }

    if( removeFromInventory ) {
        RemoveItemFromInventory( itemToEquip );
    }

    Item* currentEquipment = m_equippedItems[slot];

    // Return previous item to inventory
    if( currentEquipment != nullptr ) {
        AddItemToInventory( currentEquipment );
    }

    m_equippedItems[slot] = itemToEquip;
}


void Inventory::UnequipItem( Item* itemToUnequip ) {
    ItemSlot slot = itemToUnequip->GetItemSlot();

    if( m_equippedItems[slot] == itemToUnequip ) {
        m_equippedItems[slot] = nullptr;
        AddItemToInventory( itemToUnequip );
    }
}


void Inventory::SetRenderPreferences( bool renderEquippedItems /*= false*/, bool renderUnequippedItems /*= false */ ) {
    m_renderEquippedItems = renderEquippedItems;
    m_renderUnequippedItems = renderUnequippedItems;
}


void Inventory::AddItemSets( const Strings& validSetVec ) {
    m_itemSets.SetTags( validSetVec );
}


void Inventory::AddItemSets( const std::string& validSetCSV ) {
    m_itemSets.SetTags( validSetCSV );
}


Item* Inventory::GetItemAtPosition( const Vec2& worldPosition ) const {
     int requestedTileIndex = m_map->GetTileIndexFromWorldCoords( worldPosition );

    // Unequipped Items
    int numItems = (int)m_unequippedItems.size();
    for( int itemIndex = 0; itemIndex < numItems; itemIndex++ ) {
        Item* item = m_unequippedItems[itemIndex];

        if( item != nullptr ) {
            int itemTileIndex = m_map->GetTileIndexFromWorldCoords( item->GetPosition() );

            if( requestedTileIndex == itemTileIndex ) {
                return item;
            }
        }
    }

    // Equipped Items
    for( int itemIndex = 0; itemIndex < NUM_ITEM_SLOTS; itemIndex++ ) {
        Item* item = m_equippedItems[itemIndex];

        if( item != nullptr ) {
            int itemTileIndex = m_map->GetTileIndexFromWorldCoords( item->GetPosition() );

            if( requestedTileIndex == itemTileIndex ) {
                return item;
            }
        }
    }

    return nullptr;
}


Item* Inventory::GetItemInSlot( int unequippedSlotIndex ) const {
    int numItems = (int)m_unequippedItems.size();

    if( unequippedSlotIndex < numItems ) {
        return m_unequippedItems[unequippedSlotIndex];
    } else {
        return nullptr;
    }
}


Item* Inventory::GetItemInSlot( ItemSlot equippedItemSlot ) const {
    return m_equippedItems[equippedItemSlot];
}


bool Inventory::IsItemEquipable( const Item* itemToEquip ) const {
    ItemSlot slot = itemToEquip->GetItemSlot();

    if( slot == ITEM_SLOT_NONE ) {
        return false;
    }

    const std::vector< Tags >& setsToEquip = itemToEquip->GetItemSets();
    int numEquipSets = (int)setsToEquip.size();

    for( int setIndex = 0; setIndex < numEquipSets; setIndex++ ) {
        const Tags& equipSet = setsToEquip[setIndex];

        if( !m_itemSets.HasAtLeastOneTag( equipSet ) ) {
            return false;
        }
    }

    return true;
}
