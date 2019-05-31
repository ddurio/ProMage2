#pragma once
#include "Game/GameCommon.hpp"

/*
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntVec2.hpp"

#include "map"


class Actor;
class SpriteSheet;
class Texture;

class ActorDef {
    public:
    static void InitializeActorDefs();
    static void DestroyActorDefs();
    static const ActorDef* GetActorDef( std::string actorType );

    void Define( Actor& actor ) const;

    const std::string& GetActorType() const;
    std::string GetTexturePath() const;
    int GetPPU() const;
    const AABB2& GetUVs() const;

    private:
    explicit ActorDef( const XMLElement& element );

    static std::map<std::string, ActorDef*> s_actorDefs;

    std::string m_actorType = "";
    SpriteSheet* m_spriteSheet = nullptr;
    IntVec2 m_spriteCoords = IntVec2::ZERO;
    AABB2 m_spriteUVs = AABB2::ZEROTOONE;
    int m_spritePPU = 1;

    bool m_canSee  = true;
    bool m_canWalk = true;
    bool m_canFly  = false;
    bool m_canSwim = true;

    FloatRange m_strength     = FloatRange::ZERO;
    FloatRange m_intelligence = FloatRange::ZERO;
    FloatRange m_agility      = FloatRange::ZERO;
};
*/


// New Definition format
#include "Game/Definition.hpp"


class Actor;


template<>
Definition<Actor>::Definition( const XMLElement& element );

template<>
void Definition<Actor>::Define( Actor& theObject ) const;
