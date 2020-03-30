#include "Engine/Physics/Trigger2D.hpp"

#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/RigidBody2D.hpp"


bool Trigger2D::GetTriggerInfo( Collider2D* colliderB, TriggerInfo2D& outTriggerInfo ) {
    std::map< const Collider2D*, TriggerInfo2D >::iterator infoIter = m_triggerInfos.find( colliderB );

    // Get existing or Create new info
    if( infoIter != m_triggerInfos.end() ) {
        outTriggerInfo = infoIter->second;
    } else {
        outTriggerInfo.myTrigger = this;
        outTriggerInfo.otherCollider = colliderB;
    }

    // Check if still overlapping
    float radiusA = 0.f;
    float radiusB = 0.f;

    OBB2 boundsA = GetWorldBounds( radiusA );
    OBB2 boundsB = colliderB->GetWorldBounds( radiusB );
    Manifold2 manifold;

    if( Manifold2::GetManifold( boundsA, radiusA, boundsB, radiusB, manifold ) ) {
        int frame = m_parent->GetPhysicsFrame();
        outTriggerInfo.latestFrame = frame;

        if( infoIter == m_triggerInfos.end() ) { // First time seeing this collider
            outTriggerInfo.firstFrame = frame;
        }

        m_triggerInfos[colliderB] = outTriggerInfo;
        return true;
    }

    return false;
}


TriggerInfo2D Trigger2D::GetExistingTriggerInfo( const Collider2D* collider ) const {
    std::map< const Collider2D*, TriggerInfo2D >::const_iterator infoIter = m_triggerInfos.find( collider );

    if( infoIter != m_triggerInfos.end() ) {
        return infoIter->second;
    }

    return TriggerInfo2D();
}


Trigger2D::Trigger2D( RenderContext* renderContext, RigidBody2D* parent, const AABB2& box, float radius /*= 0.f*/, const Rgba& debugColor /*= Rgba::WHITE */ ) :
    RigidBodyChild2D( renderContext, parent, box, radius, debugColor ) {
}


Trigger2D::Trigger2D( RenderContext* renderContext, RigidBody2D* parent, const OBB2& box, float radius /*= 0.f*/, const Rgba& debugColor /*= Rgba::WHITE */ ) :
    RigidBodyChild2D( renderContext, parent, box, radius, debugColor ) {
}


Trigger2D::Trigger2D( RenderContext* renderContext, RigidBody2D* parent, const Capsule2& capsule, const Rgba& debugColor /*= Rgba::WHITE */ ) :
    RigidBodyChild2D( renderContext, parent, capsule, debugColor ) {
}


Trigger2D::Trigger2D( RenderContext* renderContext, RigidBody2D* parent, const Vec2& discCenter, float discRadius, const Rgba& debugColor /*= Rgba::WHITE */ ) :
    RigidBodyChild2D( renderContext, parent, discCenter, discRadius, debugColor ) {
}


void Trigger2D::FireCallbackEvent( RBChildEvent event, void* info ) {
    TriggerInfo2D& collision = *(TriggerInfo2D*)info;
    std::string eventName = "";

    // Find event name
    switch( event ) {
        case( RBCHILD_EVENT_ENTER ): {
            eventName = m_callbackEvents[event];
            break;
        } case( RBCHILD_EVENT_LEAVE ): {
            eventName = m_callbackEvents[event];
            
            // Remove collider from known overlaps
            const Collider2D* leavingCollider = collision.otherCollider;
            m_triggerInfos.erase( leavingCollider );
            break;
        } case( RBCHILD_EVENT_COLLIDE ): {
            std::string msg = "(Trigger2D) WARNING - Invalid callback requested of type 'Collision'";
            g_theDevConsole->PrintString( msg, DevConsole::CHANNEL_WARNING );
            return;
        } default: {
            std::string msg = Stringf( "(Trigger2D) WARNING - Unrecognized callback event index (%d)", (int)event );
            g_theDevConsole->PrintString( msg, DevConsole::CHANNEL_WARNING );
            return;
        }
    }

    // Fire event
    EventArgs args;
    args.SetValue( PHYSICS_ARG_MY_TRIGGER,      collision.myTrigger     );
    args.SetValue( PHYSICS_ARG_OTHER_COLLIDER,  collision.otherCollider );
    args.SetValue( PHYSICS_ARG_FIRST_FRAME,     collision.firstFrame    );
    args.SetValue( PHYSICS_ARG_LATEST_FRAME,    collision.latestFrame   );

    g_theEventSystem->FireEvent( eventName, args );
}


TriggerInfo2D::TriggerInfo2D( const Trigger2D* myTriggerIn, const Collider2D* otherColliderIn, int firstFrameIn, int latestFrameIn ) :
    myTrigger( myTriggerIn ),
    otherCollider( otherColliderIn ),
    firstFrame( firstFrameIn ),
    latestFrame( latestFrameIn ) {
}
