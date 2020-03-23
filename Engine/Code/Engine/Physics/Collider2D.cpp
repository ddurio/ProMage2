#define _USE_MATH_DEFINES
#include <cmath>
#include "Engine/Physics/Collider2D.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Debug/Profiler.hpp"
#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/RigidBody2D.hpp"
#include "Engine/Physics/Trigger2D.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "algorithm"


void Collider2D::SetIsColliding( bool isColliding ) {
    m_isColliding = isColliding;
}


float Collider2D::SetMomentOfInertia( float massCollider ) {
    /* Rounded OBB Subdivision
                @@@----------@@@      -
              @@@ |          | @@@    |
             @@ D |          | D @@   |
        -    @----|          |----@   |
        |    |    |          |    |   |
   2*Ey |    | B2 |    B1    | B3 |   | 2*Ey + 2*r
        |    |    |          |    |   |
        |    |    |          |    |   |
        -    @----|          |----@   |
             @@ D |          | D @@   |
              @@@ |          | @@@    |
                @@@----------@@@      -

                  |---2*Ex---|
    */

    // Dimensions
    float extentsX = m_localBounds.halfExtents.x;
    float extentsY = m_localBounds.halfExtents.y;

    float widthBox1 = (2 * extentsX);
    float heightBox1 = (2 * extentsY + 2 * m_radius);
    float widthBox2 = m_radius;
    float heightBox2 = (2 * extentsY);

    // Area
    float areaBox1 = widthBox1 * heightBox1;
    float areaBox2 = widthBox2 * heightBox2;
    float areaBox3 = areaBox2;
    float areaDisc = (float)M_PI * m_radius * m_radius;
    float areaTotal = areaBox1 + areaBox2 + areaBox3 + areaDisc;

    // Mass
    float massBox1 = massCollider * (areaBox1 / areaTotal);
    float massBox2 = massCollider * (areaBox2 / areaTotal);
    float massDisc = massCollider * (areaDisc / areaTotal);

    // Offsets from Center of Mass
    float offsetBox2 = extentsX + (0.5f * m_radius);
    float offsetDisc = Vec2( offsetBox2, extentsY + (0.5f * m_radius) ).GetLength();

    // Moment of Inertia - Box + Parallel Axis Theorem
    float momentBox1 = (1.f / 12.f) * massBox1 * ((widthBox1 * widthBox1) + (heightBox1 * heightBox1));
    float momentBox2 = (1.f / 12.f) * massBox2 * ((widthBox2 * widthBox2) + (heightBox2 * heightBox2));
    momentBox2 += (0.5f * offsetBox2) * (0.5f * offsetBox2) * massBox2;

    // Moment of Inertia - Disc + Parallel Axis Theorem
    float momentBox3 = momentBox2;
    float momentDisc = 0.5f * massDisc * m_radius * m_radius;
    momentDisc += (0.5f * offsetDisc) * (0.5f * offsetDisc) * massDisc;

    // Moment of Inertia - Total + Parallel Axis Theorem (if needed)
    float momentCollider = momentBox1 + momentBox2 + momentBox3 + momentDisc;

    if( m_localBounds.center != Vec2::ZERO ) {
        float distCollider = m_localBounds.center.GetLength();
        momentCollider += (0.5f * distCollider) * (0.5f * distCollider) * massCollider;
    }

    m_moment = momentCollider;
    return m_moment;
}


bool Collider2D::IsColliding( const Collider2D* otherCollider ) const {
    CollisionInfo2D collisionInfo;
    return GetCollisionInfo( otherCollider, collisionInfo );
}


bool Collider2D::GetIsColliding() const {
    return m_isColliding;
}


float Collider2D::GetArea() const {
    // See 'Rounded OBB Subdivision' @ SetMomentOfInertia

    // Dimensions
    float extentsX = m_localBounds.halfExtents.x;
    float extentsY = m_localBounds.halfExtents.y;

    float widthBox1 = (2 * extentsX);
    float heightBox1 = (2 * extentsY + 2 * m_radius);
    float widthBox2 = m_radius;
    float heightBox2 = (2 * extentsY);

    // Area
    float areaBox1 = widthBox1 * heightBox1;
    float areaBox2 = widthBox2 * heightBox2;
    float areaBox3 = areaBox2;
    float areaDisc = (float)M_PI * m_radius * m_radius;
    float areaTotal = areaBox1 + areaBox2 + areaBox3 + areaDisc;

    return areaTotal;
}


float Collider2D::GetMomentOfInertia() const {
    return m_moment;
}


bool Collider2D::GetCollisionInfo( const Collider2D* colliderB, CollisionInfo2D& outCollisionInfo ) const {
    PROFILE_FUNCTION();

    outCollisionInfo.myCollider = this;
    outCollisionInfo.otherCollider = colliderB;

    return GetManifold( colliderB, outCollisionInfo.manifold );
}


Collider2D::Collider2D( RenderContext* renderContext, RigidBody2D* parent, const AABB2& box, float radius /*= 0.f*/, const Rgba& debugColor /*= Rgba::WHITE */ ) :
    RigidBodyChild2D( renderContext, parent, box, radius, debugColor ) {
}

Collider2D::Collider2D( RenderContext* renderContext, RigidBody2D* parent, const OBB2& box, float radius /*= 0.f*/, const Rgba& debugColor /*= Rgba::WHITE */ ) :
    RigidBodyChild2D( renderContext, parent, box, radius, debugColor ) {
}

Collider2D::Collider2D( RenderContext* renderContext, RigidBody2D* parent, const Capsule2& capsule, const Rgba& debugColor /*= Rgba::WHITE */ ) :
    RigidBodyChild2D( renderContext, parent, capsule, debugColor ) {
}

Collider2D::Collider2D( RenderContext* renderContext, RigidBody2D* parent, const Vec2& discCenter, float discRadius, const Rgba& debugColor /*= Rgba::WHITE */ ) :
    RigidBodyChild2D( renderContext, parent, discCenter, discRadius, debugColor ) {
}


void Collider2D::FireCallbackEvent( RBChildEvent event, void* info ) {
    std::string eventName = "";

    // Find event name
    switch( event ) {
        case(RBCHILD_EVENT_COLLIDE): {
            eventName = m_callbackEvents[event];
            break;
        } case(RBCHILD_EVENT_ENTER): {
            std::string msg = "(Collider2D) WARNING - Invalid callback requested of type 'TriggerEnter'";
            g_theDevConsole->PrintString( msg, DevConsole::CHANNEL_WARNING );
            return;
        } case( RBCHILD_EVENT_LEAVE ): {
            std::string msg = "(Collider2D) WARNING - Invalid callback requested of type 'TriggerLeave'";
            g_theDevConsole->PrintString( msg, DevConsole::CHANNEL_WARNING );
            return;
        } default: {
            std::string msg = Stringf( "(Collider2D) WARNING - Unrecognized callback event index (%d)", (int)event );
            g_theDevConsole->PrintString( msg, DevConsole::CHANNEL_WARNING );
            return;
        }
    }

    // Fire event
    CollisionInfo2D& collision = *(CollisionInfo2D*)info;

    EventArgs args = m_callbackArgs;
    args.SetValue( PHYSICS_ARG_MY_COLLIDER,     collision.myCollider    );
    args.SetValue( PHYSICS_ARG_OTHER_COLLIDER,  collision.otherCollider );
    args.SetValue( PHYSICS_ARG_MANIFOLD,        collision.manifold      );

    g_theEventSystem->FireEvent( eventName, args );
}


void Collider2D::Destroy() {
    RigidBodyChild2D::Destroy();

    int numTriggers = (int)m_overlappingTriggers.size();

    for( int triggerIndex = 0; triggerIndex < numTriggers; triggerIndex++ ) {
        Trigger2D*& trigger = m_overlappingTriggers[triggerIndex];

        if( trigger != nullptr ) {
            TriggerInfo2D info = trigger->GetExistingTriggerInfo( this );
            trigger->FireCallbackEvent( RBCHILD_EVENT_LEAVE, &info );
            trigger->GetParent()->UpdateTriggerColor( trigger );
        }
    }
}


CollisionInfo2D::CollisionInfo2D( const Collider2D* myColliderIn, const Collider2D* otherColliderIn, const Manifold2& manifoldIn ) :
        myCollider( myColliderIn ),
        otherCollider( otherColliderIn ),
        manifold( manifoldIn ) {
}


CollisionInfo2D CollisionInfo2D::GetInverse() const {
    Manifold2 inverseManifold = manifold.GetInverted();
    CollisionInfo2D inverse = CollisionInfo2D( otherCollider, myCollider, inverseManifold );

    return inverse;
}


