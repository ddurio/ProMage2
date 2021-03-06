#pragma once
#include "Game/GameCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


class MGS_Sprinkle : public MapGenStep {
    friend class EditorMapGenStep;

    public:
    explicit MGS_Sprinkle( const XMLElement& element, const Strings& motifHierarchy );
    explicit MGS_Sprinkle( const Strings& motifHierarchy );
    ~MGS_Sprinkle() {};

    void SaveToXml( XmlDocument& document, XMLElement& element ) const override;
    bool RecalculateMotifVars( EventArgs& args ) override;


    private:
    IntRange m_countRange = IntRange::ONE;

    void RunOnce( Map& map ) const;
};
