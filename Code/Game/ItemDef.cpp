#include "Game/ItemDef.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/Item.hpp"
#include "Game/XMLUtils.hpp"


std::map<std::string, ItemDef*> ItemDef::s_itemDefs;

ItemDef::ItemDef( const XMLElement& element ) {
    m_itemType     = ParseXMLAttribute( element, "name",         m_itemType );
    m_itemSlot     = ParseXMLAttribute( element, "slot",         m_itemSlot );
    m_spriteCoords = ParseXMLAttribute( element, "spriteCoords", m_spriteCoords );

    g_theDevConsole->PrintString( Stringf( "(ItemDef) Loaded new ItemDef (%s)", m_itemType.c_str() ) );

    s_itemDefs[m_itemType] = this;
}


void ItemDef::InitializeItemDefs() {
    XmlDocument document = new XmlDocument();
    const XMLElement& root = ParseXMLRootElement( DATA_ITEM_DEFS, document );

    std::string imageFilePath   = ParseXMLAttribute( root, "imageFilePath", "" );
    IntVec2     imageLayout     = ParseXMLAttribute( root, "gridLayout",    IntVec2::ZERO );
    int         imagePPU        = ParseXMLAttribute( root, "ppu",           1 );

    SpriteSheet* sprites = new SpriteSheet( imageFilePath, imageLayout );

    const char* tagName = "ItemDefinition";
    const XMLElement* element = root.FirstChildElement( tagName );

    for( element; element != nullptr; element = element->NextSiblingElement( tagName ) ) {
        ItemDef* itemDef = new ItemDef( *element ); // Upon construction, adds self to static registry
        itemDef->m_spriteSheet = sprites;
        itemDef->m_imagePPU = imagePPU;

        Vec2 uvMins = Vec2::ZERO;
        Vec2 uvMaxs = Vec2::ONE;
        sprites->GetSpriteDef( itemDef->m_spriteCoords ).GetUVs(uvMins, uvMaxs);
        itemDef->m_spriteUVs = AABB2( uvMins, uvMaxs );
    }
}


void ItemDef::DestroyItemDefs() {
    auto itemDefIter = s_itemDefs.begin();
    SpriteSheet* sprites = itemDefIter->second->m_spriteSheet;
    delete sprites;

    for( itemDefIter; itemDefIter != s_itemDefs.end(); itemDefIter++ ) {
        ItemDef* itemDef = itemDefIter->second;
        delete itemDef;
        itemDefIter->second = nullptr;
    }
}


const ItemDef* ItemDef::GetItemDef( std::string itemType ) {
    auto itemDefIter = s_itemDefs.find( itemType );

    if( itemDefIter != s_itemDefs.end() ) {
        return itemDefIter->second;
    } else {
        return nullptr;
    }
}


void ItemDef::Define( Item& item ) const {
    UNUSED( item );
    // Define item specific things here.. roll the dice, etc
}


const std::string& ItemDef::GetItemType() const {
    return m_itemType;
}


const ItemSlot ItemDef::GetItemSlot() const {
    return m_itemSlot;
}


std::string ItemDef::GetTexturePath() const {
    return m_spriteSheet->GetTexturePath();
}


int ItemDef::GetPPU() const {
    return m_imagePPU;
}


const AABB2& ItemDef::GetUVs() const {
    return m_spriteUVs;
}
