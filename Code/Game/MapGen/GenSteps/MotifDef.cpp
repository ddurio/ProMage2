#include "Game/MapGen/GenSteps/MotifDef.hpp"

#include "Game/XMLUtils.hpp"


const NamedProperties& MotifDef::GetVariables() const {
    return m_variables;
}

// PRIVATE ------------------------------------------
MotifDef::MotifDef( const XMLElement& element ) {
    s_defClass = "MotifDef";

    m_defType = ParseXMLAttribute( element, "name", m_defType );
    GUARANTEE_OR_DIE( m_defType != "", "(MotifDef) Missing required attribute 'name'" );

    const XMLElement* childEle = element.FirstChildElement();

    while( childEle != nullptr ) {
        std::string varKey = childEle->Name();
        std::string varValue = ParseXMLAttribute( *childEle, "value", "" );

        if( varValue == "" ) {
            std::string warningMsg = Stringf( "(MotifDef): WARNING -- variable value unspecified for '%s'", varKey.c_str() );
            ERROR_RECOVERABLE( warningMsg.c_str() );
        } else {
            m_variables.SetValue( varKey, varValue );
        }

        childEle = childEle->NextSiblingElement();
    }
}


void MotifDef::DefineObject( Motif& theObject ) const {
    UNUSED( theObject );
}
