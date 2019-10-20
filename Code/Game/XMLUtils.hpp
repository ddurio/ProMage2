#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Utils/XMLUtils.hpp"

#include "Game/MapGen/GenSteps/MotifDef.hpp"

#include "regex"


enum ItemSlot : int;


ItemSlot ParseXMLAttribute( const XMLElement& element, const char* attributeName, ItemSlot defaultValue );
std::string ParseXMLAttribute( const XMLElement& element, const char* attributeName, const Strings& motifHeirarchy, const char* defaultValue );


template< typename T >
T ParseXMLAttribute( const XMLElement& element, const char* attributeName, const Strings& motifHeirarchy, const T& defaultValue ) {
    std::string attrValue = ParseXMLAttribute( element, attributeName, "" );

    if( attrValue == "" ) { // Not in XML at all
        return defaultValue;
    }

    std::regex xmlVariableRegex( "^%(.+)%$", std::regex_constants::icase );
    std::smatch regexMatch;

    if( regex_search( attrValue, regexMatch, xmlVariableRegex ) ) {
        std::string motifVarName = regexMatch[1];
        Strings::const_iterator motifIter = motifHeirarchy.begin();

        while( motifIter != motifHeirarchy.end() ) {
            const MotifDef* motif = MotifDef::GetDefinition( *motifIter );

            if( motif != nullptr ) {
                const NamedProperties& motifVars = motif->GetVariables();

                T motifValue = motifVars.GetValue( motifVarName, defaultValue );

                if( motifValue != defaultValue ) {
                    return motifValue;
                }
            }

            motifIter++;
        }

        std::string errorMsg = Stringf( "(XMLUtils): ERROR -- Failed to find motif with value for '%s' variable", motifVarName.c_str() );
        ERROR_RECOVERABLE( errorMsg.c_str() );
    }

    return ParseXMLAttribute( element, attributeName, defaultValue );
}


