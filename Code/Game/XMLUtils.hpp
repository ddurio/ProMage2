#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Utils/XMLUtils.hpp"

#include "Game/MapGen/GenSteps/MotifDef.hpp"

#include "regex"


enum ItemSlot : int;


ItemSlot ParseXMLAttribute( const XMLElement& element, const char* attributeName, ItemSlot defaultValue );
std::string ParseXMLAttribute( const XMLElement& element, const char* attributeName, const Strings& motifHeirarchy, const char* defaultValue );

bool GetXMLMotifVariable( const XMLElement& element, const char* attributeName, NamedStrings& out_motifVars, const std::string& attrAlternateName = "" );
bool GetXMLMotifVariable( const XMLElement& element, const char* attributeName, std::string& out_motifVarName );


template< typename T >
T ParseXMLAttribute( const XMLElement& element, const char* attributeName, const Strings& motifHeirarchy, const T& defaultValue ) {
    std::string motifVarName = "";

    if( GetXMLMotifVariable( element, attributeName, motifVarName ) ) {
        return MotifDef::GetVariableValue( motifHeirarchy, motifVarName, defaultValue );
    }

    return ParseXMLAttribute( element, attributeName, defaultValue );
}


