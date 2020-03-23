#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Utils/XMLUtils.hpp"

#include "Game/MapGen/GenSteps/MotifDef.hpp"

#include "regex"


enum ItemSlot : int;


ItemSlot ParseXMLAttribute( const XMLElement& element, const char* attributeName, ItemSlot defaultValue );
std::string ParseXMLAttribute( const XMLElement& element, const char* attributeName, NamedStrings& out_motifVars, const Strings& motifHeirarchy, const char* defaultValue, const std::string& attrAlternateName = "" );

bool GetXMLMotifVariable( const XMLElement& element, const char* attributeName, NamedStrings& out_motifVars, const std::string& attrAlternateName = "" );


template< typename T >
T ParseXMLAttribute( const XMLElement& element, const char* attributeName, NamedStrings& out_motifVars, const Strings& motifHeirarchy, const T& defaultValue, const std::string& attrAlternateName = "" ) {
    if( GetXMLMotifVariable( element, attributeName, out_motifVars, attrAlternateName ) ) {
        std::string finalAttrName = (attrAlternateName == "") ? attributeName : attrAlternateName;
        std::string motifVarName = out_motifVars.GetValue( finalAttrName, "" );
        GUARANTEE_OR_DIE( motifVarName != "", "(XMLUtils) Motif variable name should not be empty" );

        return MotifDef::GetVariableValue( motifHeirarchy, motifVarName, defaultValue );
    }

    return ParseXMLAttribute( element, attributeName, defaultValue );
}


