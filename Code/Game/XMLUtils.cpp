#include "Game/XMLUtils.hpp"

#include "Game/ItemDef.hpp"

ItemSlot ParseXMLAttribute( const XMLElement& element, const char* attributeName, ItemSlot defaultValue ) {
    std::string slotString = ParseXMLAttribute( element, attributeName, "none" );

    ItemSlot slot = defaultValue;

    if( slotString == "mainHand" ) {
        slot = ITEM_SLOT_MAIN_HAND;
    } else if( slotString == "offHand" ) {
        slot = ITEM_SLOT_OFF_HAND;
    }

    return slot;
}
