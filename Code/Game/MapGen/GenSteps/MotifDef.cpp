#include "Game/MapGen/GenSteps/MotifDef.hpp"

#include "Game/XMLUtils.hpp"


MotifDef::MotifDef( const XMLElement& element, bool addToDefList /*= false */ ) {
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

    if( addToDefList ) {
        AddDefinition( this );
    }
}


MotifDef::MotifDef( const std::string& motifNameToCopy, const std::string& nameToAppend ) {
    const MotifDef* motifToCopy = GetDefinition( motifNameToCopy );

    m_defType = Stringf( "%s_%s", motifToCopy->m_defType.c_str(), nameToAppend.c_str() );
    m_variables = motifToCopy->m_variables;

    AddDefinition( this );
}


const NamedProperties& MotifDef::GetVariables() const {
    return m_variables;
}


NamedProperties& MotifDef::GetVariables() {
    return m_variables;
}


std::string MotifDef::GetVariableValue( const Strings& motifHierarchy, const std::string& varName, const char* defaultValue ) {
    return GetVariableValue( motifHierarchy, varName, std::string( defaultValue ) );
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
void MotifDef::DefineObject( Motif& theObject ) const {
    UNUSED( theObject );
}
