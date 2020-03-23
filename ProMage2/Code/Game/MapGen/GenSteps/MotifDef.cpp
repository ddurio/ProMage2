#include "Game/MapGen/GenSteps/MotifDef.hpp"

#include "Game/XMLUtils.hpp"


MotifDef::MotifDef( const XMLElement& element, bool addToDefList /*= false */ ) {
    s_defClass = "MotifDef";

    m_defType = ParseXMLAttribute( element, "name", m_defType );
    GUARANTEE_OR_DIE( m_defType != "", "(MotifDef) Missing required attribute 'name'" );

    const char* defaultValue = "__HOPEFULLY_NOT_A_VALUE__";
    const XMLElement* childEle = element.FirstChildElement();

    while( childEle != nullptr ) {
        std::string varName = childEle->Name();
        std::string varValue = ParseXMLAttribute( *childEle, "value", defaultValue );

        if( varValue == defaultValue ) {
            std::string warningMsg = Stringf( "(MotifDef): WARNING -- variable value unspecified for '%s'", varName.c_str() );
            ERROR_RECOVERABLE( warningMsg.c_str() );
        } else {
            m_variableValues.SetValue( varName, varValue );
        }

        std::string varType = ParseXMLAttribute( *childEle, "type", "" );
        m_variableTypes.SetValue( varName, varType );

        childEle = childEle->NextSiblingElement();
    }

    if( addToDefList ) {
        AddDefinition( this );
    }
}


MotifDef::MotifDef( const std::string& motifNameToCopy, const std::string& nameToAppend ) {
    const MotifDef* motifToCopy = GetDefinition( motifNameToCopy );

    if( motifToCopy != nullptr ) {
        m_defType = Stringf( "%s_%s", motifToCopy->m_defType.c_str(), nameToAppend.c_str() );
        m_variableValues = NamedProperties( motifToCopy->m_variableValues );
        m_variableTypes = motifToCopy->m_variableTypes;

        AddDefinition( this );
    }
}


const NamedProperties& MotifDef::GetVariableValues() const {
    return m_variableValues;
}


NamedProperties& MotifDef::GetVariableValues() {
    return m_variableValues;
}


const NamedStrings& MotifDef::GetVariableTypes() const {
    return m_variableTypes;
}


std::string MotifDef::GetVariableValue( const Strings& motifHierarchy, const std::string& varName, const char* defaultValue ) {
    return GetVariableValue( motifHierarchy, varName, std::string( defaultValue ) );
}


std::string MotifDef::GetVariableType( const Strings& motifHierarchy, const std::string& varName, const std::string& defaultValue ) {
    Strings::const_iterator motifIter = motifHierarchy.begin();

    while( motifIter != motifHierarchy.end() ) {
        const MotifDef* motif = MotifDef::GetDefinition( *motifIter );

        if( motif != nullptr ) {
            const NamedProperties& motifVarValues = motif->GetVariableValues();
            const NamedStrings& motifVarTypes = motif->GetVariableTypes();

            if( motifVarValues.IsNameSet( varName ) ) {
                return motifVarTypes.GetValue( varName, defaultValue );
            }
        }

        motifIter++;
    }

    return defaultValue;
}


Strings MotifDef::GetVariableNames( const Strings& motifHierarchy ) {
    Strings hierarchyVars;
    int numMotifs = (int)motifHierarchy.size();

    for( int motifIndex = 0; motifIndex < numMotifs; motifIndex++ ) {
        const std::string& motifName = motifHierarchy[motifIndex];
        const MotifDef* motif = MotifDef::GetDefinition( motifName );

        if( motif != nullptr ) {
            const NamedProperties& variables = motif->GetVariableValues();
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
