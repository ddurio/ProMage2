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


std::string ParseXMLAttribute( const XMLElement& element, const char* attributeName, NamedStrings& out_motifVars, const Strings& motifHeirarchy, const char* defaultValue, const std::string& attrAlternateName /*= "" */ ) {
    return ParseXMLAttribute( element, attributeName, out_motifVars, motifHeirarchy, std::string( defaultValue ), attrAlternateName );
}


bool GetXMLMotifVariable( const XMLElement& element, const char* attributeName, NamedStrings& out_motifVars, const std::string& attrAlternateName /*= "" */ ) {
    std::string attrValue = ParseXMLAttribute( element, attributeName, "" );

    std::regex xmlVariableRegex( "^%(.+)%$", std::regex_constants::icase );
    std::smatch regexMatch;

    if( regex_search( attrValue, regexMatch, xmlVariableRegex ) ) {
        std::string storageName = (attrAlternateName == "") ? attributeName : attrAlternateName;
        out_motifVars.SetValue( storageName, regexMatch[1] );
        return true;
    }

    return false;
}
