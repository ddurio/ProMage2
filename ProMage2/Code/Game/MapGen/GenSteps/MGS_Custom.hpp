#pragma once
#include "Game/GameCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


class MGS_CustomDef;


class MGS_Custom : public MapGenStep {
    friend class EditorMapGenStep;
    friend class MGS_CustomDef;

    public:
    explicit MGS_Custom( const XMLElement& element, const Strings& motifHierarchy );
    explicit MGS_Custom( const std::string& stepType, const Strings& motifHierarchy );
    ~MGS_Custom() {};

    std::vector< CustomEvent > GetCustomResults() const override;

    void SaveToXml( XmlDocument& document, XMLElement& element ) const override;
    void UpdateParentMotifs( const Strings& parentMotifHierarchy ) override;
    bool RecalculateMotifVars( EventArgs& args ) override;


    private:
    static int s_numCustomSteps;

    std::string m_defType = "";
    std::vector< MapGenStep* > m_genSteps;

    MotifDef* m_customXmlMotif = nullptr; // NON-const version for editor


    void RunOnce( Map& theMap ) const;

    void SetupEmptyMotif();
    void SetupMotifFromXML( const XMLElement& element );
};

