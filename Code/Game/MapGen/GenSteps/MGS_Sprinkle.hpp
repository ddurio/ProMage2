#pragma once
#include "Game/GameCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


class MGS_Sprinkle : public MapGenStep {
    friend class EditorMapGenStep;

    public:
    explicit MGS_Sprinkle( const XMLElement& element );
    ~MGS_Sprinkle() {};

    void SaveToXml( XmlDocument& document, XMLElement& element ) const override;


    private:
    IntRange m_countRange = IntRange::ZERO;

    void RunOnce( Map& map ) const;
};
