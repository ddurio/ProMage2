#pragma once
#include "Game/GameCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


class MGS_CustomDef;


class MGS_Custom : public MapGenStep {
    friend class EditorMapGenStep;
    friend class MGS_CustomDef;

    public:
    MGS_Custom( const XMLElement& element, const Strings& motifHierarchy );
    ~MGS_Custom() {};

    void SaveToXml( XmlDocument& document, XMLElement& element ) const override;
    bool RecalculateMotifVars( EventArgs& args ) override;


    private:
    static int s_numCustomSteps;

    std::string m_defType = "";
    std::vector< MapGenStep* > m_genSteps;

    NamedProperties m_virtualParams;


    void RunOnce( Map& theMap ) const;

    void SetupMotifFromXML( const XMLElement& element, const MGS_CustomDef* virtualDef );
};

