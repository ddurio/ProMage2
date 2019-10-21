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


std::string ParseXMLAttribute( const XMLElement& element, const char* attributeName, const Strings& motifHeirarchy, const char* defaultValue ) {
    return ParseXMLAttribute( element, attributeName, motifHeirarchy, std::string( defaultValue ) );
}


bool GetXMLMotifVariable( const XMLElement& element, const char* attributeName, NamedStrings& motifVars ) {
    std::string attrValue = ParseXMLAttribute( element, attributeName, "" );

    std::regex xmlVariableRegex( "^%(.+)%$", std::regex_constants::icase );
    std::smatch regexMatch;

    if( regex_search( attrValue, regexMatch, xmlVariableRegex ) ) {
        motifVars.SetValue( attributeName, regexMatch[1] );
        return true;
    }

    return false;
}


bool GetXMLMotifVariable( const XMLElement& element, const char* attributeName, std::string& out_motifVarName ) {
    std::string attrValue = ParseXMLAttribute( element, attributeName, "" );

    std::regex xmlVariableRegex( "^%(.+)%$", std::regex_constants::icase );
    std::smatch regexMatch;

    if( regex_search( attrValue, regexMatch, xmlVariableRegex ) ) {
        out_motifVarName = regexMatch[1];
        return true;
    }

    return false;
}
