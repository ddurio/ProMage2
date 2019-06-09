#include "Game/Animator.hpp"

#include "Engine/Core/Timer.hpp"
#include "Engine/Renderer/IsoSpriteAnimDef.hpp"

#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/GameState.hpp"


Animator::Animator( Actor* myActor ) :
    m_myActor( myActor ) {
    GameState* state = g_theGame->GetGameState();
    m_animTimer = new Timer( state->GetStateClock() );
    m_animTimer->Start( 1.f );

    m_currentAnim = IsoSpriteAnimDef::GetDefinition( ANIM_PAPER_DOLL_IDLE );
}


Animator::~Animator() {
    CLEAR_POINTER( m_animTimer );
}


void Animator::Update( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    Vec2 moveDir = m_myActor->GetMoveDir();
    std::string animName = ANIM_PAPER_DOLL_IDLE;

    if( !m_myActor->IsAlive() ) {
        animName = ANIM_PAPER_DOLL_DIE;
    } else if( moveDir != Vec2::ZERO ) {
        animName = ANIM_PAPER_DOLL_WALK;
    }

    const IsoSpriteAnimDef* newAnim = IsoSpriteAnimDef::GetDefinition( animName );

    if( m_currentAnim != newAnim ) {
        m_currentAnim = newAnim;
        m_animTimer->Restart();
    }

    m_prevMoveDir = (moveDir == Vec2::ZERO) ? m_prevMoveDir : moveDir; // Keep last facing if you're not moving now
}


const SpriteDef& Animator::GetCurrentSpriteDef() const {
    float elapsedTime = m_animTimer->GetElapsedTime();

    return m_currentAnim->GetSpriteDefAtTimeAndDirection( elapsedTime, m_prevMoveDir );
}


const SpriteDef& Animator::GetPortraitSpriteDef() const {
    const IsoSpriteAnimDef* idleAnim = IsoSpriteAnimDef::GetDefinition( ANIM_PAPER_DOLL_IDLE );

    return idleAnim->GetSpriteDefAtTimeAndDirection( 0.f, Vec2::DOWN );
}
