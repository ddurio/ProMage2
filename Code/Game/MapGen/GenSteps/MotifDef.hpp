#pragma once
#include "Editor/EditorCommon.hpp"

#include "Engine/Utils/Definition.hpp"
#include "Engine/Utils/NamedStrings.hpp"


class Motif; // Doesn't need to exist so long as no one calls defineObject


class MotifDef : public Definition< Motif, MotifDef > {
    friend class Definition< Motif, MotifDef >;

    public:
    const NamedProperties& GetVariables() const;

    template< typename T >
    static T GetVariableValue( const Strings& motifHierarchy, const std::string& varName, T defaultValue );
    static std::string GetVariableValue( const Strings& motifHierarchy, const std::string& varName, const char* defaultValue );
    static Strings GetVariableNames( const Strings& motifHierarchy );


    private:
    NamedProperties m_variables;


    MotifDef( const XMLElement& element );

    void DefineObject( Motif& theObject ) const override;
};


template< typename T >
T MotifDef::GetVariableValue( const Strings& motifHierarchy, const std::string& varName, T defaultValue ) {
    Strings::const_iterator motifIter = motifHierarchy.begin();

    while( motifIter != motifHierarchy.end() ) {
        const MotifDef* motif = MotifDef::GetDefinition( *motifIter );

        if( motif != nullptr ) {
            const NamedProperties& motifVars = motif->GetVariables();

            if( motifVars.IsNameSet( varName ) ) {
                return motifVars.GetValue( varName, defaultValue );
            }
        }

        motifIter++;
    }

    return defaultValue;
}
