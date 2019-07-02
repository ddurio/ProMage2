#include "Game/Inventory.hpp"

#include "Engine/Core/DebugDraw.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteAnimDef.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/TextureView2D.hpp"

#include "Game/Item.hpp"
#include "Game/Map.hpp"


Inventory::Inventory( Actor* owner, Map*& theMap, bool renderEquippedItems /*= true*/, bool renderUnequippedItems /*= false */ ) :
    m_owner( owner ),
    m_map( theMap ) {
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


void Inventory::TransferMoney( int moneyToTransfer ) {
    // moneyToTransfer could be negative to remove money
    m_money += moneyToTransfer;
}


void Inventory::AddItemToInventory( Item* itemToAdd, int indexToAdd /*= -1 */ ) {
    // DFS1FIXME: Check for empty equipped slot of the same type and auto equip instead
    if( indexToAdd >= 0 ) {
        // Could potentially overwrite an item.. good if intentional
        m_unequippedItems[indexToAdd] = itemToAdd;
        return;
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
        if( !removeFromInventory ) {
            AddItemToInventory( itemToEquip, currentItemDestinationIndex );
        }

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


bool Inventory::HasAllItemSets( const Tags& itemSetTags ) const {
    return m_itemSets.HasTags( itemSetTags );
}


bool Inventory::HasOneItemSet( const Tags& itemSetTags ) const {
    return m_itemSets.HasAtLeastOneTag( itemSetTags );
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


Item* Inventory::GetClosestItemInRange( const Vec2& worldPosition, float pickupRadius ) const {
    float radiusSquared = pickupRadius * pickupRadius;
    int bestIndex = -1;
    float bestDistSquared = -1.f;

    for( int itemIndex = 0; itemIndex < m_numItemSlots; itemIndex++ ) {
        Item* item = m_unequippedItems[itemIndex];

        if( item != nullptr ) {
            Vec2 position = item->GetPosition();
            float distSquared = (position - worldPosition).GetLengthSquared();

            if( bestIndex == -1 || distSquared < bestDistSquared ) {
                bestIndex = itemIndex;
                bestDistSquared = distSquared;
            }
        }
    }

    if( bestDistSquared < radiusSquared ) {
        return m_unequippedItems[bestIndex];
    } else {
        return nullptr;
    }
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


int Inventory::GetMoney() const {
    return m_money;
}


float Inventory::GetDefense() const {
    float defense = 0.f;

    for( int itemIndex = 0; itemIndex < NUM_ITEM_SLOTS - 1; itemIndex++ ) {
        Item* item = m_equippedItems[itemIndex];

        if( item != nullptr ) {
            defense += item->GetDefense();
        }
    }

    return defense;
}


void Inventory::AddUIContent() {
    // Setup backpack view
    for( int itemIndex = 0; itemIndex < m_numItemSlots; itemIndex++ ) {
        AddUnequippedItemTile( itemIndex );
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
    CreateMoneyTile( tileSize );
    ImGui::Image( nullptr, tileSize );
    AddEquippedItemTile( ITEM_SLOT_WEAPON, tileSize );
    ImGui::Image( nullptr, tileSize );

    style.ItemSpacing.y *= 0.25f;
}


void Inventory::UpdateUI() {
    // Separated to enable the trade window where someone else sets up the window
    // but inventory can still easily add the backpack/equipment items
    CreateUIWindow();
    AddUIContent();
    ImGui::End();
}


void Inventory::CreateUIWindow() {
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
}


void Inventory::AddUnequippedItemTile( int itemIndex ) {
    ImGuiStyle& style = ImGui::GetStyle();

    float tileWidth = (ImGui::GetColumnWidth() - (11.f * style.ItemSpacing.x)) * 0.1f; // Assumed 10 tiles per row
    ImVec2 tileSize = ImVec2( tileWidth, tileWidth );

    // Create Button
    CreateItemTile( itemIndex, false, tileSize );

    // Add New Line
    if( itemIndex + 1 < m_numItemSlots && (itemIndex % 10) != 9 ) {
        ImGui::SameLine();
    }
}


void Inventory::AddEquippedItemTile( ItemSlot itemSlot, const ImVec2& tileSize ) {
    std::string emptyTileName = ANIM_INVENTORY_EMPTY;

    switch( itemSlot ) {
        case( ITEM_SLOT_CHEST ):    { emptyTileName = ANIM_INVENTORY_CHEST;     break; }
        case( ITEM_SLOT_FEET ):     { emptyTileName = ANIM_INVENTORY_FEET;      break; }
        case( ITEM_SLOT_HELM ):     { emptyTileName = ANIM_INVENTORY_HELM;      break; }
        case( ITEM_SLOT_LEGS ):     { emptyTileName = ANIM_INVENTORY_LEGS;      break; }
        case( ITEM_SLOT_SHOULDER ): { emptyTileName = ANIM_INVENTORY_SHOULDER;  break; }
        case( ITEM_SLOT_WEAPON ):   { emptyTileName = ANIM_INVENTORY_WEAPON;    break; }
    }

    CreateItemTile( itemSlot, true, tileSize, emptyTileName );
}


struct ItemTilePayload {
    int itemIndex = -1;
    bool isEquipped = false;
    Inventory* myInventory = nullptr;


    ItemTilePayload( int index, bool equipped, Inventory* inventory ) :
        itemIndex( index ),
        isEquipped( equipped ),
        myInventory( inventory ) {
    };
};


void Inventory::CreateItemTile( int itemIndex, bool isEquipped, const ImVec2& tileSize, const std::string& emptyTileName /*= ANIM_INVENTORY_EMPTY */ ) {
    void* shaderResourceView = nullptr;
    AABB2 uvs;

    Item* item = isEquipped ? m_equippedItems[itemIndex] : m_unequippedItems[itemIndex];
    bool itemWasConsumed = false;

    // Create tile based on Item
    if( item != nullptr ) {
        ImGui::PushID( item );
        const SpriteDef& portrait = item->GetPortrait();

        TextureView2D* textureView = g_theRenderer->GetOrCreateTextureView2D( portrait.GetTexturePath() );
        shaderResourceView = textureView->GetShaderView();

        portrait.GetUVs( uvs.mins, uvs.maxs );
        
        ImVec4 qualityColor = item->GetQualityColor().GetAsImGui();
        ImGui::ImageButton( shaderResourceView, tileSize, ImVec2( uvs.mins.x, uvs.maxs.y ), ImVec2( uvs.maxs.x, uvs.mins.y ), 0, qualityColor );

        if( ImGui::IsItemHovered() ) {
            CreateItemTooltip( item ); // Tooltip on hover

            // Consumable
            if( item->IsConsumable() && ImGui::IsMouseDoubleClicked( 0 ) ) {
                item->Consume( m_owner );

                RemoveItemFromInventory( item );
                CLEAR_POINTER( item );
                itemWasConsumed = true;
            }
        }
    } else { // Create empty tile
        const SpriteAnimDef* slotAnim = SpriteAnimDef::GetDefinition( emptyTileName );
        const SpriteDef sprite = slotAnim->GetSpriteDefAtTime( 0.f );

        TextureView2D* textureView = g_theRenderer->GetOrCreateTextureView2D( sprite.GetTexturePath() );
        shaderResourceView = textureView->GetShaderView();

        sprite.GetUVs( uvs.mins, uvs.maxs );

        ImGui::ImageButton( shaderResourceView, tileSize, ImVec2( uvs.mins.x, uvs.maxs.y ), ImVec2( uvs.maxs.x, uvs.mins.y ), 0 );
    }

    SetupDragAndDrop( item, itemIndex, isEquipped, shaderResourceView, tileSize, uvs );

    if( item != nullptr || itemWasConsumed ) {
        ImGui::PopID();
    }
}


void Inventory::CreateItemTooltip( Item* item ) const {
    Actor* player = m_map->GetPlayer();
    Inventory* playerInventory = player->GetInventory();

    ImGuiStyle& tooltipStyle = ImGui::GetStyle();
    ImGuiStyle origStyle = tooltipStyle;

    // Item tooltip: name, slot, etc
    ImGui::BeginTooltip();

    ImGui::Text( "Name: %s", item->GetItemType().c_str() );

    ItemSlot slot = item->GetItemSlot();
    if( slot != ITEM_SLOT_NONE ) {
        ImGui::Text( "Slot: %s", item->GetItemSlotText().c_str() );

        if( slot == ITEM_SLOT_WEAPON ) {
            ImGui::Text( "Damage: %.0f", item->GetAttackDamage() );
        } else {
            ImGui::Text( "Defense: %.0f", item->GetDefense() );
        }
    }

    // Required item sets
    std::vector< Tags > requiredSets = item->GetItemSets();
    int numSets = (int)requiredSets.size();

    for( int setIndex = 0; setIndex < numSets; setIndex++ ) {
        Tags& set = requiredSets[setIndex];
        Strings tags = set.GetTags();

        
        tooltipStyle.Colors[ImGuiCol_Text] = (playerInventory->HasOneItemSet( set )) ? Rgba::GREEN.GetAsImGui() : Rgba::RED.GetAsImGui();
        ImGui::Text( "Requires: %s", JoinStrings( tags, " OR " ).c_str() );
    }

    // Consumable description
    if( item->IsConsumable() ) {
        std::string description = item->GetConsumptionDescription();

        ImGui::Text( "%s", description.c_str() );
        ImGui::Text( "Consumable (double click)" );
    }

    // Quality
    tooltipStyle.Colors[ImGuiCol_Text] = item->GetQualityColor().GetAsImGui();
    std::string qualityDesc = item->GetQualityDescription();
    ImGui::Text( "%s", qualityDesc.c_str() );

    // Money Value
    tooltipStyle.Colors[ImGuiCol_Text] = Rgba::WHITE.GetAsImGui();
    std::string moneyStr = Stringf( "Sells for: %d", item->GetValue() );
    ImGui::Text( moneyStr.c_str() );

    ImGui::EndTooltip();
    tooltipStyle = origStyle;
}


void Inventory::CreateMoneyTile( const ImVec2& tileSize ) {
    TextureView2D* textureView = g_theRenderer->GetOrCreateTextureView2D( TEXTURE_GOLD_PILE );
    void* shaderResourceView = textureView->GetShaderView();

    // Center the text (and calc size in the process)
    ImGuiStyle& style = ImGui::GetStyle();

    std::string moneyStr = Stringf( "%d", m_money );
    ImVec2 textSize = ImGui::CalcTextSize( moneyStr.c_str() );
    float columnWidth = ImGui::GetColumnWidth();

    float centeredTextStart = (columnWidth - textSize.x) * 0.5f - style.ItemSpacing.x;
    float currentPosX = ImGui::GetCursorPosX();

    // Scale tile down by text size
    ImVec2 scaledTileSize;
    scaledTileSize.x = tileSize.x - (2.f * textSize.y); // scale evenly by height of text
    scaledTileSize.y = tileSize.y - (2.f * textSize.y);

    // Draw it all
    float centeredImageStart = (columnWidth - scaledTileSize.x) * 0.5f - style.ItemSpacing.x;
    ImGui::SetCursorPosX( currentPosX + centeredImageStart );
    ImGui::Image( shaderResourceView, scaledTileSize );

    ImGui::SetCursorPosX( currentPosX + centeredTextStart );
    ImGui::Text( moneyStr.c_str() );
}


void Inventory::SetupDragAndDrop( Item* item, int itemIndex, bool isEquipped, void* shaderResourceView, const ImVec2& tileSize, const AABB2& uvs ) {
    // Setup Drag
    if( (item != nullptr ) && ImGui::BeginDragDropSource( ImGuiDragDropFlags_None ) ) {
        ItemTilePayload sourcePayload = ItemTilePayload( itemIndex, isEquipped, this );

        ImGui::SetDragDropPayload( "ItemTile", &sourcePayload, sizeof( ItemTilePayload ) );
        ImGui::Image( shaderResourceView, tileSize, ImVec2( uvs.mins.x, uvs.maxs.y ), ImVec2( uvs.maxs.x, uvs.mins.y ) );
        //ImGui::Text( "%x", this );

        ImGui::EndDragDropSource();
    }

    // Setup Drop
    if( ImGui::BeginDragDropTarget() ) {
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "ItemTile" );

        if( payload != nullptr ) {
            IM_ASSERT( payload->DataSize == sizeof( ItemTilePayload ) );

            ItemTilePayload sourcePayload = *(const ItemTilePayload*)payload->Data;
            ItemTilePayload targetPayload = ItemTilePayload( itemIndex, isEquipped, this );

            HandleDragAndDrop( sourcePayload, targetPayload );
        }

        ImGui::EndDragDropTarget();
    }
}


void Inventory::HandleDragAndDrop( const ItemTilePayload& sourcePayload, const ItemTilePayload& targetPayload ) {
    Inventory* sourceInv = sourcePayload.myInventory;
    Inventory* targetInv = targetPayload.myInventory;

    //std::string sourceStr = Stringf( "(%d, %sequipped)", sourcePayload.itemIndex, sourcePayload.isEquipped ? "" : "un" );
    //std::string targetStr = Stringf( "(%d, %sequipped)", targetPayload.itemIndex, targetPayload.isEquipped ? "" : "un" );

    if( sourceInv == targetInv ) {
        //std::string msg = Stringf( "Swap Items: %s, %s", sourceStr.c_str(), targetStr.c_str() );
        //g_theDebugger->DrawDebugMessage( msg, 10.f );
        sourceInv->SwapItems( sourcePayload, targetPayload );
    } else {
        //std::string msg = Stringf( "Trade Items: %s, %s", sourceStr.c_str(), targetStr.c_str() );
        //g_theDebugger->DrawDebugMessage( msg, 10.f );
        TradeItems( sourcePayload, targetPayload );
    }
}


// Only for swapping items within the same inventory
void Inventory::SwapItems( const ItemTilePayload& sourcePayload, const ItemTilePayload& targetPayload ) {
    Inventory* sourceInv = sourcePayload.myInventory;
    Inventory* targetInv = targetPayload.myInventory;

    if( sourceInv != targetInv && this == sourceInv ) {
        g_theDevConsole->PrintString( "(Inventory) WARNING: Inventory pointers must match to swap items", DevConsole::CHANNEL_WARNING );
        return;
    }


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


void Inventory::TradeItems( const ItemTilePayload& sourcePayload, const ItemTilePayload& targetPayload ) {
    Inventory* sourceInv = sourcePayload.myInventory;
    Inventory* targetInv = targetPayload.myInventory;

    if( sourceInv == targetInv ) {
        g_theDevConsole->PrintString( "(Inventory) WARNING: Inventory pointers must differ to trade items", DevConsole::CHANNEL_WARNING );
        return;
    }


    // Setup Inventory & Item pointers
    Item* sourceItem = nullptr;
    Item* targetItem = nullptr;

    int sourceIndex = sourcePayload.itemIndex;
    int targetIndex = targetPayload.itemIndex;

    bool sourceIsEquipped = sourcePayload.isEquipped;
    bool targetIsEquipped = targetPayload.isEquipped;


    if( sourceIndex >= 0 ) {
        if( sourceIsEquipped ) {
            sourceItem = sourceInv->m_equippedItems[sourceIndex];
        } else {
            sourceItem = sourceInv->m_unequippedItems[sourceIndex];
        }
    }

    if( targetIndex >= 0 ) {
        if( targetIsEquipped ) {
            targetItem = targetInv->m_equippedItems[targetIndex];
        } else {
            targetItem = targetInv->m_unequippedItems[targetIndex];
        }
    }


    // Check Gold Value of Trade
    if( !IsTradeValid(sourceItem, sourceInv, targetItem, targetInv) ) {
        return;
    }


    // Actually trade the items (and gold if needed)
    if( sourceIsEquipped ) {
        if( !targetIsEquipped ) {
            if( targetItem == nullptr ) { // Unequip source, move to target
                ItemSlot sourceSlot = sourceItem->GetItemSlot();

                sourceInv->m_equippedItems[sourceSlot] = nullptr;
                targetInv->m_unequippedItems[targetIndex] = sourceItem;

                TradeMoney( sourceItem, sourceInv, targetItem, targetInv );
            } else {
                ItemSlot sourceSlot = sourceItem->GetItemSlot();
                ItemSlot targetSlot = targetItem->GetItemSlot();

                if( sourceSlot == targetSlot ) { // Slots match
                    // Equip from different inventory
                    sourceInv->m_equippedItems[sourceSlot] = nullptr;

                    sourceInv->EquipItem( targetItem, false );
                    targetInv->m_unequippedItems[targetIndex] = sourceItem;

                    TradeMoney( sourceItem, sourceInv, targetItem, targetInv );
                }
            }
        } else { // Both equipped
            ItemSlot sourceSlot = (ItemSlot)sourceIndex; // only true because it's equipped
            ItemSlot targetSlot = (ItemSlot)targetIndex;

            if( sourceSlot == targetSlot ) { // Both equip from different inventories
                sourceInv->m_equippedItems[sourceSlot] = nullptr;
                targetInv->m_equippedItems[targetSlot] = nullptr; // don't let the items unequip into wrong backpack

                targetInv->EquipItem( sourceItem, false );

                if( targetItem != nullptr ) {
                    sourceInv->EquipItem( targetItem, false );
                }

                TradeMoney( sourceItem, sourceInv, targetItem, targetInv );
            }
        }
    } else { // Source unequipped
        if( targetIsEquipped ) {
            ItemSlot sourceSlot = sourceItem->GetItemSlot();
            ItemSlot targetSlot = (ItemSlot)targetIndex;

            if( sourceSlot == targetSlot ) {
                targetInv->m_equippedItems[targetSlot] = nullptr;
                targetInv->EquipItem( sourceItem, false );
                sourceInv->m_unequippedItems[sourceIndex] = targetItem;

                TradeMoney( sourceItem, sourceInv, targetItem, targetInv );
            }
        } else {
            sourceInv->m_unequippedItems[sourceIndex] = targetItem;
            targetInv->m_unequippedItems[targetIndex] = sourceItem;

            TradeMoney( sourceItem, sourceInv, targetItem, targetInv );
        }
    }
}


void Inventory::TradeMoney( const Item* sourceItem, Inventory* sourceInv, const Item* targetItem, Inventory* targetInv ) {
    int sourceValue = sourceItem->GetValue();
    int targetValue = (targetItem != nullptr) ? targetItem->GetValue() : 0;

    int moneyAmount = sourceValue - targetValue;

    sourceInv->TransferMoney(  moneyAmount );
    targetInv->TransferMoney( -moneyAmount );
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


bool Inventory::IsTradeValid( const Item* sourceItem, const Inventory* sourceInv, const Item* targetItem, const Inventory* targetInv ) {
    int sourceValue = sourceItem->GetValue();
    int sourceMoney = sourceInv->GetMoney();
    int sourceTotal = sourceValue + sourceMoney;

    int targetValue = (targetItem != nullptr) ? targetItem->GetValue() : 0;
    int targetMoney = targetInv->GetMoney();
    int targetTotal = targetValue + targetMoney;

    bool sourceCanTrade = (targetTotal >= sourceValue);
    bool targetCanTrade = (sourceTotal >= targetValue);

    return sourceCanTrade && targetCanTrade;
}
