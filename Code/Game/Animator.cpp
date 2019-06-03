#include "Game/Animator.hpp"

#include "Engine/Core/Timer.hpp"
#include "Engine/Renderer/IsoSpriteAnimDef.hpp"

#include "Game/Actor.hpp"
#include "Game/Game.hpp"


Animator::Animator( Actor* myActor ) :
    m_myActor( myActor ) {
    m_animTimer = new Timer( g_theGame->GetGameClock() );
    m_animTimer->Start( 1.f );

    m_currentAnim = IsoSpriteAnimDef::GetDefinition( PAPER_DOLL_ANIM_IDLE );
}


Animator::~Animator() {
    CLEAR_POINTER( m_animTimer );
}


void Animator::Update( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    Vec2 moveDir = m_myActor->GetMoveDir();
    std::string animName = PAPER_DOLL_ANIM_IDLE;

    if( !m_myActor->IsAlive() ) {
        animName = PAPER_DOLL_ANIM_DIE;
    } else if( moveDir != Vec2::ZERO ) {
        animName = PAPER_DOLL_ANIM_WALK;
    }

    m_currentAnim = IsoSpriteAnimDef::GetDefinition( animName );
    m_prevMoveDir = (moveDir == Vec2::ZERO) ? m_prevMoveDir : moveDir; // Keep last facing if you're not moving now
}


const SpriteDef& Animator::GetCurrentSpriteDef() const {
    float elapsedTime = m_animTimer->GetElapsedTime();

    return m_currentAnim->GetSpriteDefAtTimeAndDirection( elapsedTime, m_prevMoveDir );
}
