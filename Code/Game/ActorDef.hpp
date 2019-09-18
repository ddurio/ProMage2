#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Utils/Definition.hpp"


class Actor;
class StatsManager;


class ActorDef : public Definition< Actor, ActorDef > {
    friend class Definition< Actor, ActorDef >;

    public:
    void DefineObject( Actor& theObject ) const override;

    float GetSpritePPU() const;

    private:
    bool m_canSee = true;
    bool m_canWalk = true;
    bool m_canFly = false;
    bool m_canSwim = false;
    bool m_canDie = true;

    Strings m_parentActors;
    Strings m_bodySprites;
    Strings m_earSprites;
    Strings m_hairSprites;
    float m_spritePPU = 40.f;

    Strings m_bodyItemSets;
    std::string m_validItemSets;

    const StatsManager* m_statsManager = nullptr;
    int m_money = 0;


    ActorDef( const XMLElement& element );
    ~ActorDef();
};

