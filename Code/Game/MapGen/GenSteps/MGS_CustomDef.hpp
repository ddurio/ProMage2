#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Utils/Definition.hpp"


class MapGenStep;
class MGS_Custom;


class MGS_CustomDef : public Definition< MGS_Custom, MGS_CustomDef > {
    friend class Definition< MGS_Custom, MGS_CustomDef >;

    public:
    void DefineObject( MGS_Custom& theObject ) const override;

    std::string GetMotif() const;


    private:
    std::vector< MapGenStep* > m_genSteps;
    std::string m_motif = "";


    MGS_CustomDef( const XMLElement& element );
    ~MGS_CustomDef();
};
