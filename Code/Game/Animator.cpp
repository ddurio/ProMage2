#include "Game/Animator.hpp"

#include "Engine/Core/Timer.hpp"
#include "Engine/Renderer/IsoSpriteAnimDef.hpp"

#include "Game/Actor.hpp"
#include "Game/ActorController.hpp"
#include "Game/Inventory.hpp"
#include "Game/Item.hpp"
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
    } else if( m_myActor->GetAttackTarget() != nullptr ) {
        Inventory* myInventory = m_myActor->GetInventory();
        Item* weapon = myInventory->GetItemInSlot( ITEM_SLOT_WEAPON );

        animName = weapon->GetAttackAnim();
    }

    const IsoSpriteAnimDef* newAnim = IsoSpriteAnimDef::GetDefinition( animName );

    if( m_currentAnim != newAnim ) {
        m_currentAnim = newAnim;
        m_animTimer->Restart();
        m_prevElapsedTime = 0.f;
    }

    m_prevMoveDir = (moveDir == Vec2::ZERO) ? m_prevMoveDir : moveDir; // Keep last facing if you're not moving now
    TriggerAnimEvents();
}


const SpriteDef& Animator::GetCurrentSpriteDef() const {
    float elapsedTime = m_animTimer->GetElapsedTime();

    return m_currentAnim->GetSpriteDefAtTimeAndDirection( elapsedTime, m_prevMoveDir );
}


const SpriteDef& Animator::GetPortraitSpriteDef() const {
    const IsoSpriteAnimDef* idleAnim = IsoSpriteAnimDef::GetDefinition( ANIM_PAPER_DOLL_IDLE );

    return idleAnim->GetSpriteDefAtTimeAndDirection( 0.f, Vec2::DOWN );
}


void Animator::TriggerAnimEvents() {
    float elapsedTime = m_animTimer->GetElapsedTime();

    Strings events = m_currentAnim->GetEventsInTimeRangeAndDirection( m_prevElapsedTime, elapsedTime, m_prevMoveDir );
    int numEvents = (int)events.size();

    for( int eventIndex = 0; eventIndex < numEvents; eventIndex++ ) {
        std::string& eventStr = events[eventIndex];
        Strings eventSplitStr = SplitStringOnDelimeter( eventStr, ' ', false );

        const std::string& command = eventSplitStr[0];

        /*
        if( StringICmp( command, "dealDamage" ) ) {
            m_myUnit->DealDamage( m_attackVictim );
        } else */
        if( StringICmp( command, "playSound" ) ) {
            GUARANTEE_OR_DIE( (int)eventSplitStr.size() > 1, "(Animator) PlaySound anim event missing required sound name parameter" );

            SoundID soundID = g_theAudio->GetOrCreateSound( eventSplitStr[1], true );
            g_theAudio->PlaySoundAt( soundID, Vec3( m_myActor->GetPosition(), 0.f ) );
        } else if( StringICmp( command, "death" ) ) {
            const ActorController* controller = m_myActor->GetController();
            std::string deathEvent = controller->GetDeathEvent();

            g_theEventSystem->FireEvent( deathEvent );
        }
    }

    m_prevElapsedTime = elapsedTime;
}
