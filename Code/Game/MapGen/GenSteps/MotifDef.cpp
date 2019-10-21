#include "Game/MapGen/GenSteps/MotifDef.hpp"

#include "Game/XMLUtils.hpp"


const NamedProperties& MotifDef::GetVariables() const {
    return m_variables;
}


Strings MotifDef::GetVariableNames( const Strings& motifHierarchy ) {
    Strings hierarchyVars;
    int numMotifs = (int)motifHierarchy.size();

    for( int motifIndex = 0; motifIndex < numMotifs; motifIndex++ ) {
        const std::string& motifName = motifHierarchy[motifIndex];
        const MotifDef* motif = MotifDef::GetDefinition( motifName );

        if( motif != nullptr ) {
            const NamedProperties& variables = motif->GetVariables();
            Strings motifVars = variables.GetNames();
            int numVars = (int)motifVars.size();

            for( int varIndex = 0; varIndex < numVars; varIndex++ ) {
                const std::string& varName = motifVars[varIndex];

                if( !EngineCommon::VectorContains( hierarchyVars, varName ) ) {
                    hierarchyVars.push_back( varName );
                }
            }
        }
    }

    return hierarchyVars;
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
