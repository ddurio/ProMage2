#include "Game/Inventory.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteAnimDef.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/TextureView2D.hpp"

#include "Game/Item.hpp"
#include "Game/Map.hpp"


Inventory::Inventory( Map* map, bool renderEquippedItems /*= false*/, bool renderUnequippedItems /*= false*/ ) :
    m_map(map) {
    SetRenderPreferences( renderEquippedItems, renderUnequippedItems );

    m_unequippedItems.resize( m_numItemSlots );
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

    if( m_isOpen ) {
        UpdateUI();
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


void Inventory::AddItemToInventory( Item* itemToAdd, int indexToAdd /*= -1 */ ) {
    // DFS1FIXME: Check for empty equipped slot of the same type and auto equip instead
    if( indexToAdd >= 0 ) {
        Item* item = m_unequippedItems[indexToAdd];

        if( item == nullptr ) {
            m_unequippedItems[indexToAdd] = itemToAdd;
            return;
        }
    }


    int numItems = (int)m_unequippedItems.size();

    for( int itemIndex = 0; itemIndex < numItems; itemIndex++ ) {
        Item* item = m_unequippedItems[itemIndex];

        if( item == nullptr ) {
            m_unequippedItems[itemIndex] = itemToAdd;
            return;
        }
    }

    // No nullptr slots found, pushing to end
    // DFS1FIXME: Handle more than inventory size
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


void Inventory::DropItem( Item* itemToDrop ) {
    for( int itemIndex = 0; itemIndex < m_numItemSlots; itemIndex++ ) {
        const Item* item = m_unequippedItems[itemIndex];

        if( item == itemToDrop ) {
            DropItem( itemIndex );
            return;
        }
    }
}


void Inventory::DropItem( int itemIndexToDrop ) {
    Inventory* mapInventory = m_map->GetMapInventory();

    Item* itemToDrop = m_unequippedItems[itemIndexToDrop];
    m_unequippedItems[itemIndexToDrop] = nullptr;

    if( itemToDrop != nullptr ) {
        mapInventory->AddItemToInventory( itemToDrop );
    }
}


void Inventory::EquipItem( Item* itemToEquip, bool removeFromInventory /*= true*/, int currentItemDestinationIndex /*= -1 */ ) {
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
        AddItemToInventory( currentEquipment, currentItemDestinationIndex );
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


void Inventory::SetInventorySize( int numItemSlots ) {
    if( numItemSlots < m_numItemSlots ) { // Remove slots
        for( int itemIndex = numItemSlots; itemIndex < m_numItemSlots; itemIndex++ ) {
            // DFS1FIXME: Should probably look for empty slots before dropping the items
            DropItem( itemIndex );
        }
    }

    m_numItemSlots = numItemSlots;
    m_unequippedItems.resize( m_numItemSlots );
}


void Inventory::ToggleInventory() {
    m_isOpen = !m_isOpen;
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


int Inventory::GetItemIndex( Item* itemToFind, bool& outIsEquipped ) const {
    for( int itemIndex = 0; itemIndex < m_numItemSlots; itemIndex++ ) {
        Item* unequippedItem = m_unequippedItems[itemIndex];

        if( unequippedItem == itemToFind ) {
            outIsEquipped = false;
            return itemIndex;
        }
    }

    ItemSlot itemSlotToFind = itemToFind->GetItemSlot();
    Item* equippedItem = m_equippedItems[itemSlotToFind];

    if( equippedItem == itemToFind ) {
        outIsEquipped = true;
        return itemSlotToFind;
    }

    return -1;
}


void Inventory::UpdateUI() {
    // Setup Window
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    AABB2 clientBounds = g_theWindow->GetClientBounds();
    Vec2 clientDimensions = clientBounds.GetDimensions();
    clientDimensions.y = -clientDimensions.y;

    Vec2 clientCenter = clientBounds.GetCenter();

    Vec2 windowSize = 0.75f * clientDimensions;
    Vec2 windowOrigin = clientCenter - (0.5f * windowSize);

    ImGui::SetNextWindowPos( ImVec2( windowOrigin.x, windowOrigin.y ), ImGuiCond_Always );
    ImGui::SetNextWindowSize( ImVec2( windowSize.x, windowSize.y ), ImGuiCond_Always );
    ImGui::Begin( "Inventory", nullptr, windowFlags );


    // Setup Columns
    ImGui::Columns( 4, nullptr, false );
    ImGui::SetColumnWidth( 0, 0.75f * windowSize.x );

    float equipmentSize = 0.25f * windowSize.x;
    float equipmentColSize = 0.333f * equipmentSize;
    ImGui::SetColumnWidth( 1, equipmentColSize );
    ImGui::SetColumnWidth( 2, equipmentColSize );
    ImGui::SetColumnWidth( 3, equipmentColSize );

    // Setup backpack view
    for( int itemIndex = 0; itemIndex < m_numItemSlots; itemIndex++ ) {
        ImGui::PushID( itemIndex );

        AddUnequippedItemTile( itemIndex );

        ImGui::PopID();
    }

    ImGui::NextColumn();
    ImGui::Separator(); // Column separator

    // Setup equipment view
    ImGuiStyle& style = ImGui::GetStyle();
    style.ItemSpacing.y *= 4.f;

    float tileWidth = (ImGui::GetColumnWidth() - (2.f * style.ItemSpacing.x));
    ImVec2 tileSize = ImVec2( tileWidth, tileWidth );

    ImGui::Image( nullptr, tileSize );
    AddEquippedItemTile( ITEM_SLOT_SHOULDER, tileSize );
    ImGui::NextColumn();

    AddEquippedItemTile( ITEM_SLOT_HELM, tileSize );
    AddEquippedItemTile( ITEM_SLOT_CHEST, tileSize );
    AddEquippedItemTile( ITEM_SLOT_LEGS, tileSize );
    AddEquippedItemTile( ITEM_SLOT_FEET, tileSize );

    ImGui::NextColumn();
    ImGui::Image( nullptr, tileSize );
    ImGui::Image( nullptr, tileSize );
    AddEquippedItemTile( ITEM_SLOT_WEAPON, tileSize );

    style.ItemSpacing.y *= 0.25f;
    ImGui::End();
}


void Inventory::AddUnequippedItemTile( int itemIndex ) {
    float windowMaxX = ImGui::GetWindowPos().x + ImGui::GetColumnWidth();
    ImGuiStyle& style = ImGui::GetStyle();

    float tileWidth = (ImGui::GetColumnWidth() - (11.f * style.ItemSpacing.x)) * 0.1f; // Assumed 10 tiles per row
    ImVec2 tileSize = ImVec2( tileWidth, tileWidth );

    // Create Button
    CreateItemTile( itemIndex, false, tileSize );

    // Add New Line
    float thisButtonMaxX = ImGui::GetItemRectMax().x;
    float nextButtonMaxX = thisButtonMaxX + style.ItemSpacing.x + tileWidth; // Expected position if next button was on same line

    if( itemIndex + 1 < m_numItemSlots && nextButtonMaxX < windowMaxX ) {
        ImGui::SameLine();
    }
}


void Inventory::AddEquippedItemTile( ItemSlot itemSlot, const ImVec2& tileSize ) {
    std::string emptyTileName = ANIM_INVENTORY_EMPTY;

    switch( itemSlot ) {
        case( ITEM_SLOT_CHEST ): {
            emptyTileName = ANIM_INVENTORY_CHEST;
            break;
        } case( ITEM_SLOT_FEET ): {
            emptyTileName = ANIM_INVENTORY_FEET;
            break;
        } case( ITEM_SLOT_HELM ): {
            emptyTileName = ANIM_INVENTORY_HELM;
            break;
        } case( ITEM_SLOT_LEGS ): {
            emptyTileName = ANIM_INVENTORY_LEGS;
            break;
        } case( ITEM_SLOT_SHOULDER ): {
            emptyTileName = ANIM_INVENTORY_SHOULDER;
            break;
        } case( ITEM_SLOT_WEAPON ): {
            emptyTileName = ANIM_INVENTORY_WEAPON;
            break;
        }
    }

    CreateItemTile( itemSlot, true, tileSize, emptyTileName );
}


struct ItemTilePayload {
    int itemIndex = -1;
    bool isEquipped = false;

    ItemTilePayload( int index, bool equipped ) : itemIndex( index ), isEquipped( equipped ) {};
};


void Inventory::CreateItemTile( int itemIndex, bool isEquipped, const ImVec2& tileSize, const std::string& emptyTileName /*= ANIM_INVENTORY_EMPTY */ ) {
    // Create Button
    void* shaderResourceView = nullptr;
    AABB2 uvs;

    Item* item = isEquipped ? m_equippedItems[itemIndex] : m_unequippedItems[itemIndex];

    if( item != nullptr ) {
        const SpriteDef* portrait = item->GetPortrait();

        TextureView2D* textureView = g_theRenderer->GetOrCreateTextureView2D( portrait->GetTexturePath() );
        shaderResourceView = textureView->GetShaderView();

        portrait->GetUVs( uvs.mins, uvs.maxs );

        ImGui::ImageButton( shaderResourceView, tileSize, ImVec2( uvs.mins.x, uvs.maxs.y ), ImVec2( uvs.maxs.x, uvs.mins.y ), 0 );
    } else {
        const SpriteAnimDef* slotAnim = SpriteAnimDef::GetDefinition( emptyTileName );
        const SpriteDef sprite = slotAnim->GetSpriteDefAtTime( 0.f );

        TextureView2D* textureView = g_theRenderer->GetOrCreateTextureView2D( sprite.GetTexturePath() );
        shaderResourceView = textureView->GetShaderView();

        sprite.GetUVs( uvs.mins, uvs.maxs );

        //ImGui::Button( Stringf( "%s%d", "Box", itemIndex ).c_str(), tileSize );
        ImGui::ImageButton( shaderResourceView, tileSize, ImVec2( uvs.mins.x, uvs.maxs.y ), ImVec2( uvs.maxs.x, uvs.mins.y ), 0 );
    }

    // Setup Drag
    if( (item != nullptr ) && ImGui::BeginDragDropSource( ImGuiDragDropFlags_None ) ) {
        // DFS1FIXME: Update drag&drop payload to new struct
        ItemTilePayload sourcePayload = ItemTilePayload( itemIndex, isEquipped );

        ImGui::SetDragDropPayload( "ItemTile", &sourcePayload, sizeof( ItemTilePayload ) );
        ImGui::Image( shaderResourceView, tileSize, ImVec2( uvs.mins.x, uvs.maxs.y ), ImVec2( uvs.maxs.x, uvs.mins.y ) );

        ImGui::EndDragDropSource();
    }

    // Setup Drop
    if( ImGui::BeginDragDropTarget() ) {
        if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "ItemTile" ) ) {
            IM_ASSERT( payload->DataSize == sizeof( ItemTilePayload ) );

            ItemTilePayload sourcePayload = *(const ItemTilePayload*)payload->Data;
            ItemTilePayload targetPayload = ItemTilePayload( itemIndex, isEquipped );

            SwapItems( sourcePayload, targetPayload );
        }

        ImGui::EndDragDropTarget();
    }
}


void Inventory::SwapItems( const ItemTilePayload& sourcePayload, const ItemTilePayload& targetPayload ) {
    Item* sourceItem = nullptr;
    Item* targetItem = nullptr;

    if( sourcePayload.itemIndex >= 0 ) {
        sourceItem = sourcePayload.isEquipped ? m_equippedItems[sourcePayload.itemIndex] : m_unequippedItems[sourcePayload.itemIndex];
    }

    if( targetPayload.itemIndex >= 0 ) {
        targetItem = targetPayload.isEquipped ? m_equippedItems[targetPayload.itemIndex] : m_unequippedItems[targetPayload.itemIndex];
    }


    if( sourcePayload.isEquipped ) {
        if( !targetPayload.isEquipped ) {
            if( targetItem == nullptr ) {
                ItemSlot sourceSlot = sourceItem->GetItemSlot();

                m_equippedItems[sourceSlot] = nullptr;
                m_unequippedItems[targetPayload.itemIndex] = sourceItem;
            } else {
                ItemSlot sourceSlot = sourceItem->GetItemSlot();
                ItemSlot targetSlot = targetItem->GetItemSlot();

                if( sourceSlot == targetSlot ) {
                    EquipItem( targetItem, true, targetPayload.itemIndex );
                }
            }
        }
    } else {
        if( targetPayload.isEquipped ) {
            ItemSlot sourceSlot = sourceItem->GetItemSlot();
            ItemSlot targetSlot = (ItemSlot)targetPayload.itemIndex; // only true because it's equipped

            if( sourceSlot == targetSlot ) {
                EquipItem( sourceItem, true, sourcePayload.itemIndex );
            }
        } else {
            m_unequippedItems[sourcePayload.itemIndex] = targetItem;
            m_unequippedItems[targetPayload.itemIndex] = sourceItem;
        }
    }
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
