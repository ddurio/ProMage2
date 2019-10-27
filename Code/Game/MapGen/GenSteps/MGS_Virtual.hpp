#pragma once
#include "Game/GameCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


class MGS_VirtualDef;


class MGS_Virtual : public MapGenStep {
    friend class EditorMapGenStep;
    friend class MGS_VirtualDef;

    public:
    MGS_Virtual( const XMLElement& element, const Strings& motifHierarchy );
    ~MGS_Virtual() {};

    void SaveToXml( XmlDocument& document, XMLElement& element ) const override;
    bool RecalculateMotifVars( EventArgs& args ) override;


    private:
    static int s_numVirtualSteps;

    std::string m_defType = "";
    std::vector< MapGenStep* > m_genSteps;

    NamedProperties m_virtualParams;


    void RunOnce( Map& theMap ) const;

    void SetupMotifFromXML( const XMLElement& element, const MGS_VirtualDef* virtualDef );
};

