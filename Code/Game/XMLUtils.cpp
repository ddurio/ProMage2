#include "Game/XMLUtils.hpp"

#include "Game/ItemDef.hpp"


ItemSlot ParseXMLAttribute( const XMLElement& element, const char* attributeName, ItemSlot defaultValue ) {
    std::string slotString = ParseXMLAttribute( element, attributeName, "none" );
    slotString = StringToLower( slotString );

    ItemSlot slot = defaultValue;

    if( slotString == "helm" ) {
        slot = ITEM_SLOT_HELM;
    } else if( slotString == "chest" ) {
        slot = ITEM_SLOT_CHEST;
    } else if( slotString == "shoulder" ) {
        slot = ITEM_SLOT_SHOULDER;
    } else if( slotString == "legs" ) {
        slot = ITEM_SLOT_LEGS;
    } else if( slotString == "feet" ) {
        slot = ITEM_SLOT_FEET;
    } else if( slotString == "weapon" ) {
        slot = ITEM_SLOT_WEAPON;
    }

    return slot;
}
