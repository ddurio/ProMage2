#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Utils/Definition.hpp"


class MapGenStep;
class MGS_Virtual;


class MGS_VirtualDef : public Definition< MGS_Virtual, MGS_VirtualDef > {
    friend class Definition< MGS_Virtual, MGS_VirtualDef >;

    public:
    void DefineObject( MGS_Virtual& theObject ) const override;

    std::string GetMotif() const;


    private:
    std::vector< MapGenStep* > m_genSteps;
    std::string m_motif = "";


    MGS_VirtualDef( const XMLElement& element );
    ~MGS_VirtualDef();
};
