#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Math/Vec2.hpp"


class Actor;
class IsoSpriteAnimDef;
class SpriteDef;
class Timer;


class Animator {
    public:
    Animator( Actor* myActor );
    ~Animator();

    void Update( float deltaSeconds );

    const SpriteDef& GetCurrentSpriteDef() const;

    private:
    Actor* m_myActor = nullptr;

    const IsoSpriteAnimDef* m_currentAnim = nullptr;
    Timer* m_animTimer = nullptr;

    Vec2 m_prevMoveDir = Vec2::DOWN;
};
