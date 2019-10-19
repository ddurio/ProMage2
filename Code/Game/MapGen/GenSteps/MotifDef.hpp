#pragma once
#include "Editor/EditorCommon.hpp"

#include "Engine/Utils/Definition.hpp"
#include "Engine/Utils/NamedStrings.hpp"


class Motif;


class MotifDef : public Definition< Motif, MotifDef > {
    friend class Definition< Motif, MotifDef >;

    public:
    std::string GetValue( const std::string& key, const std::string& defaultValue ) const;


    private:
    NamedStrings m_variables;


    MotifDef( const XMLElement& element );

    void DefineObject( Motif& theObject ) const override;
};
