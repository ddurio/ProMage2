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

    std::string GetCurrentAnimName() const;
    const SpriteDef& GetCurrentSpriteDef() const;
    const SpriteDef& GetPortraitSpriteDef() const;
    Vec2 GetCurrentFacing() const;
    bool AnimHasFinished() const;

    private:
    Actor* m_myActor = nullptr;

    std::string m_currentAnimName = "";
    const IsoSpriteAnimDef* m_currentAnim = nullptr;
    Timer* m_animTimer = nullptr;

    Vec2 m_prevMoveDir = Vec2::DOWN;
    float m_prevElapsedTime = 0.f;


    void TriggerAnimEvents();
};
