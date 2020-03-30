#include "Engine/Physics/RigidBody2D.hpp"

#include "Engine/Debug/DebugDraw.hpp"
#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Physics/Trigger2D.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Vertex/VertexUtils.hpp"


const Rgba& RigidBody2D::DEBUG_COLOR_STATIC                    = Rgba::ORGANIC_YELLOW;
const Rgba& RigidBody2D::DEBUG_COLOR_STATIC_COLLIDING          = Rgba::ORGANIC_PURPLE;
const Rgba& RigidBody2D::DEBUG_COLOR_DYNAMIC                   = Rgba::ORGANIC_BLUE;
const Rgba& RigidBody2D::DEBUG_COLOR_DYNAMIC_COLLIDING         = Rgba::ORGANIC_RED;

const Rgba& RigidBody2D::DEBUG_COLOR_TRIGGER_STATIC            = Rgba::GRAY;
const Rgba& RigidBody2D::DEBUG_COLOR_TRIGGER_STATIC_COLLIDING  = Rgba::WHITE;
const Rgba& RigidBody2D::DEBUG_COLOR_TRIGGER_DYNAMIC           = Rgba::GRAY;
const Rgba& RigidBody2D::DEBUG_COLOR_TRIGGER_DYNAMIC_COLLIDING = Rgba::WHITE;


void RigidBody2D::BeginFrame() {
    m_frameForces = Vec2::ZERO;
    m_frameTorque = 0.f;
}


void RigidBody2D::Update( float deltaSeconds ) {
    UpdatePosition();

    if( m_simMode == SIMULATION_MODE_DYNAMIC ) {
        // Linear
        Vec2 acceleration = m_physicsSystem->GetGravity() * m_gravityScale; // Start with gravity
        acceleration += m_frameForces / GetMass(); // Add frame acceleration
        acceleration += -m_velocity * GetLinearDrag(); // Add linear drag
        m_velocity += acceleration * deltaSeconds;

        m_velocity.x = m_xIsFixed ? 0.f : m_velocity.x; // X & Y Constraints
        m_velocity.y = m_yIsFixed ? 0.f : m_velocity.y;

        m_transform.position += m_velocity * deltaSeconds;

        // Angular
        if( !m_rotIsFixed ) { // Rotational constraint
            float angularAcceleration = m_frameTorque / GetMomentOfInertia(); // Start with frame torque
            angularAcceleration += -m_angularVelocity * GetAngularDrag(); // Add angular drag
            m_angularVelocity += angularAcceleration * deltaSeconds;

            m_transform.rotationDegrees += m_angularVelocity * deltaSeconds;
        }
    }
}


void RigidBody2D::UpdatePosition() {
    m_transform = *m_gameObjectTransform;
}


void RigidBody2D::ClearCollision() {
    int numColliders = (int)m_colliders.size();

    for( int colliderIndex = 0; colliderIndex < numColliders; colliderIndex++ ) {
        Collider2D* collider = m_colliders[colliderIndex];
        collider->SetIsColliding( false );
    }

    if( m_autoUpdateColor ) {
        AutoUpdateDebugColor();
    }
}


void RigidBody2D::CheckCollision( RigidBody2D* otherRB, bool resolveCollision ) {
    if( (otherRB == m_ignoreObject) || (this == otherRB->m_ignoreObject) ) {
        return;
    }

    int numColliders = (int)m_colliders.size();

    std::vector<Collider2D*>& otherColliders = otherRB->m_colliders;
    int numOtherColliders = (int)otherColliders.size();

    // My colliders
    for( int colliderIndex = 0; colliderIndex < numColliders; colliderIndex++ ) {
        Collider2D* myCollider = m_colliders[colliderIndex];

        if( (myCollider != nullptr) && (!myCollider->m_isGarbage) ) {
            // Other rigidBody's colliders
            for( int otherColliderIndex = 0; otherColliderIndex < numOtherColliders; otherColliderIndex++ ) {
                Collider2D* otherCollider = otherColliders[otherColliderIndex];

                if( (otherCollider != nullptr) && (!otherCollider->m_isGarbage) ) { // Both colliders are valid
                    CompareColliders( myCollider, otherCollider, resolveCollision );
                }
            }
        }
    }
}


void RigidBody2D::CheckTriggers( RigidBody2D* otherRB ) {
    if( (otherRB == m_ignoreObject) || (this == otherRB->m_ignoreObject) ) {
        return;
    }

    // Only compare against dynamic colliders (but either static or dynamic triggers)
    if( otherRB->m_simMode != SIMULATION_MODE_STATIC ) {
        CompareTriggerList( otherRB->m_colliders );
    }

    if( m_simMode != SIMULATION_MODE_STATIC ) {
        otherRB->CompareTriggerList( m_colliders );
    }
}


void RigidBody2D::UpdateGameObjectTransform() {
    *m_gameObjectTransform = m_transform;
}


void RigidBody2D::RenderDebug() const {
    // Render point for rigid body
    Matrix44 model = Matrix44::MakeZRotationDegrees( m_transform.rotationDegrees );
    model.SetTranslation( m_transform.position );

    m_renderContext->BindTexture();
    m_renderContext->DrawMesh( m_mesh, model );

    // Render colliders
    int numColliders = (int)m_colliders.size();

    for( int colliderIndex = 0; colliderIndex < numColliders; colliderIndex++ ) {
        if( m_colliders[colliderIndex] != nullptr ) {
            m_colliders[colliderIndex]->RenderDebug();
        }
    }

    // Render triggers
    int numTriggers = (int)m_triggers.size();

    for( int triggerIndex = 0; triggerIndex < numTriggers; triggerIndex++ ) {
        if( m_triggers[triggerIndex] != nullptr ) {
            m_triggers[triggerIndex]->RenderDebug();
        }
    }
}


SimulationMode RigidBody2D::GetSimulationMode() const {
    return m_simMode;
}


Rgba RigidBody2D::GetDebugColor() const {
    return m_debugColor;
}


float RigidBody2D::GetMass() const {
    if( m_simMode == SIMULATION_MODE_STATIC ) {
        return std::numeric_limits<float>::infinity();
    }

    return m_mass;
}


float RigidBody2D::GetRestitution() const {
    return m_restitution;
}


float RigidBody2D::GetFriction() const {
    return m_friction;
}


float RigidBody2D::GetMomentOfInertia() const {
    if( m_simMode == SIMULATION_MODE_STATIC || m_rotIsFixed ) {
        return std::numeric_limits<float>::infinity();
    }

    return m_moment;
}


float RigidBody2D::GetLinearDrag() const {
    return m_linearDrag;
}


float RigidBody2D::GetAngularDrag() const {
    return m_angularDrag;
}


Vec2 RigidBody2D::GetVelocity() const {
    return m_velocity;
}


Vec2 RigidBody2D::GetImpactVelocity( const Vec2& worldPos ) const {
    Vec2 displacement = worldPos - m_transform.position;
    return m_velocity + (ConvertDegreesToRadians( m_angularVelocity ) * displacement.GetRotated90Degrees());
}


float RigidBody2D::GetAngularVelocity() const {
    return m_angularVelocity;
}


bool RigidBody2D::GetXConstraint() const {
    return m_xIsFixed;
}


bool RigidBody2D::GetYConstraint() const {
    return m_yIsFixed;
}


bool RigidBody2D::GetRotationConstraint() const {
    return m_rotIsFixed;
}


const std::vector<Collider2D*>& RigidBody2D::GetColliders() const {
    return m_colliders;
}


void* RigidBody2D::GetGameObject() const {
    return m_gameObject;
}


const Transform2D& RigidBody2D::GetTransform() const {
    return m_transform;
}


int RigidBody2D::GetPhysicsFrame() const {
    return m_physicsSystem->GetPhysicsFrame();
}


void RigidBody2D::SetSimulationMode( SimulationMode newMode ) {
    if( m_simMode == newMode ) {
        return;
    }

    SimulationMode oldMode = m_simMode;
    m_simMode = newMode;
    m_physicsSystem->UpdateRigidBodyMode( this, oldMode, newMode );

    if( m_autoUpdateColor ) {
        AutoUpdateDebugColor();
    }
}


Collider2D* RigidBody2D::AddCollider( const AABB2& localBounds, float radius /*= 0*/ ) {
    Rgba color = m_autoUpdateColor ? GetAutoColliderColor( false ) : m_debugColor;
    Collider2D* collider = new Collider2D( m_renderContext, this, localBounds, radius, color );
    AddColliderToList(collider);

    return collider;
}


Collider2D* RigidBody2D::AddCollider( const OBB2& localBounds, float radius /*= 0*/ ) {
    Rgba color = m_autoUpdateColor ? GetAutoColliderColor( false ) : m_debugColor;
    Collider2D* collider = new Collider2D( m_renderContext, this, localBounds, radius, color );
    AddColliderToList( collider );

    return collider;
}


Collider2D* RigidBody2D::AddCollider( const Capsule2& localBounds ) {
    Rgba color = m_autoUpdateColor ? GetAutoColliderColor( false ) : m_debugColor;
    Collider2D* collider = new Collider2D( m_renderContext, this, localBounds, color );
    AddColliderToList( collider );

    return collider;
}


Collider2D* RigidBody2D::AddCollider( const Vec2& discLocalCenter, float discRadius ) {
    Rgba color = m_autoUpdateColor ? GetAutoColliderColor( false ) : m_debugColor;
    Collider2D* collider = new Collider2D( m_renderContext, this, discLocalCenter, discRadius, color );
    AddColliderToList( collider );

    return collider;
}


Trigger2D* RigidBody2D::AddTrigger( const AABB2& localBounds, float radius /*= 0*/ ) {
    Rgba color = m_autoUpdateColor ? GetAutoTriggerColor( false ) : m_debugColor;
    Trigger2D* trigger = new Trigger2D( m_renderContext, this, localBounds, radius, color );
    AddTriggerToList(trigger);

    return trigger;
}


Trigger2D* RigidBody2D::AddTrigger( const OBB2& localBounds, float radius /*= 0*/ ) {
    Rgba color = m_autoUpdateColor ? GetAutoTriggerColor( false ) : m_debugColor;
    Trigger2D* trigger = new Trigger2D( m_renderContext, this, localBounds, radius, color );
    AddTriggerToList(trigger);

    return trigger;
}


Trigger2D* RigidBody2D::AddTrigger( const Capsule2& localBounds ) {
    Rgba color = m_autoUpdateColor ? GetAutoTriggerColor( false ) : m_debugColor;
    Trigger2D* trigger = new Trigger2D( m_renderContext, this, localBounds, color );
    AddTriggerToList(trigger);

    return trigger;
}


Trigger2D* RigidBody2D::AddTrigger( const Vec2& discLocalCenter, float discRadius ) {
    Rgba color = m_autoUpdateColor ? GetAutoTriggerColor( false ) : m_debugColor;
    Trigger2D* trigger = new Trigger2D( m_renderContext, this, discLocalCenter, discRadius, color );
    AddTriggerToList(trigger);

    return trigger;
}


void RigidBody2D::DestroyCollider( Collider2D* collider ) {
    RigidBody2D* parent = collider->GetParent();

    if( parent != this ) {
        g_theDevConsole->PrintString( "(RigidBody2D) WARNING - Attempting to destroy Collider belonging to different RigidBody!", DevConsole::CHANNEL_WARNING );
        return;
    }

    collider->Destroy();
}


void RigidBody2D::AddForce( const Vec2& newForce ) {
    m_frameForces += newForce;
}


void RigidBody2D::AddTorque( float torqueDegrees ) {
    m_frameTorque += torqueDegrees;
}


void RigidBody2D::AddImpulse( const Vec2& impulse, const Vec2& contactPos, bool ignoreMass /*= false */ ) {
    if( m_simMode == SIMULATION_MODE_STATIC ) {
        return;
    }

    // Linear
    m_velocity += (ignoreMass ? impulse : (impulse / GetMass() ) );

    // Angular
    Vec2 comToContact = contactPos - m_transform.position;
    float torqueRadians = CrossProductLength( comToContact, impulse );
    float torqueDegrees = ConvertRadiansToDegrees( torqueRadians );
    m_angularVelocity += (ignoreMass ? torqueDegrees : (torqueDegrees / GetMomentOfInertia() ) );
}


void RigidBody2D::SetGameObject( void* gameObject, Transform2D* gameObjectTransform ) {
    m_gameObject = gameObject;
    m_gameObjectTransform = gameObjectTransform;

    UpdatePosition();
}


void RigidBody2D::SetIgnoreObject( RigidBody2D* ignoreObject ) {
    m_ignoreObject = ignoreObject;
}


void RigidBody2D::SetDebugColor( const Rgba& color ) {
    if( color != m_debugColor ) {
        m_debugColor = color;
        BuildMesh();
    }

    int numColliders = (int)m_colliders.size();

    for( int colliderIndex = 0; colliderIndex < numColliders; colliderIndex++ ) {
        Collider2D* collider = m_colliders[colliderIndex];

        if( collider != nullptr ) {
            collider->SetDebugColor( color );
        }
    }
}


void RigidBody2D::SetAutoDebugColor( bool autoUpdateColor ) {
    m_autoUpdateColor = autoUpdateColor;
}


void RigidBody2D::SetGravityScale( const Vec2& gravityScale ) {
    m_gravityScale = gravityScale;
}


void RigidBody2D::SetMass( float mass ) {
    if( m_mass != mass ) {
        m_mass = mass;
        UpdateMomentOfInertia();
    }
}


void RigidBody2D::SetRestitution( float restitution ) {
    m_restitution = restitution;
}


void RigidBody2D::SetFriction( float friction ) {
    m_friction = friction;
}


void RigidBody2D::SetLinearDrag( float linearDrag ) {
    m_linearDrag = linearDrag;
}


void RigidBody2D::SetAngularDrag( float angularDrag ) {
    m_angularDrag = angularDrag;
}


void RigidBody2D::SetConstraints( bool xIsFixed, bool yIsFixed, bool rotIsFixed ) {
    m_xIsFixed = xIsFixed;
    m_yIsFixed = yIsFixed;
    m_rotIsFixed = rotIsFixed;
}


void RigidBody2D::Translate( const Vec2& translation ) {
    m_transform.position += translation;
}


void RigidBody2D::UpdateTriggerColor( Trigger2D* trigger ) {
    if( trigger->GetParent() != this ) {
        return;
    }

    // Update trigger debug color
    if( m_autoUpdateColor ) {
        int numColliding = (int)trigger->m_triggerInfos.size();
        Rgba newColor = GetAutoTriggerColor( numColliding > 0 );
        trigger->SetDebugColor( newColor );
    }
}


RigidBody2D::RigidBody2D( PhysicsSystem* physicsSystem, float mass, const Rgba& debugColor /*= Rgba::WHITE */ ) :
    m_physicsSystem(physicsSystem),
    m_mass(mass),
    m_debugColor(debugColor),
    m_renderContext(m_physicsSystem->GetRenderContext()){
    BuildMesh();
}


RigidBody2D::~RigidBody2D() {
    CLEAR_POINTER( m_mesh );
    int numColliders = (int)m_colliders.size();
    int numTriggers = (int)m_triggers.size();

    // Clear colliders
    for( int objectIndex = 0; objectIndex < numColliders; objectIndex++ ) {
        CLEAR_POINTER( m_colliders[objectIndex] );
    }

    // Clear triggers
    for( int objectIndex = 0; objectIndex < numTriggers; objectIndex++ ) {
        CLEAR_POINTER( m_triggers[objectIndex] );
    }

    m_colliders.clear();
    m_triggers.clear();
}


void RigidBody2D::Destroy() {
    m_isGarbage = true;

    // Destroy colliders
    int numColliders = (int)m_colliders.size();

    for( int colliderIndex = 0; colliderIndex < numColliders; colliderIndex++ ) {
        Collider2D* collider = m_colliders[colliderIndex];

        if( collider != nullptr ) {
            collider->Destroy();
        }
    }

    // Destroy triggers
    int numTriggers = (int)m_triggers.size();

    for( int triggerIndex = 0; triggerIndex < numTriggers; triggerIndex++ ) {
        Trigger2D* trigger = m_triggers[triggerIndex];

        if( trigger != nullptr ) {
            trigger->Destroy();
        }
    }
}

void RigidBody2D::BuildMesh() {
    if( m_mesh != nullptr ) {
        delete m_mesh;
        m_mesh = nullptr;
    }

    CPUMesh builder;
    builder.SetColor( m_debugColor );
    builder.AddCircle( Vec2::ZERO, 0.05f );

    m_mesh = new GPUMesh( m_renderContext );
    m_mesh->CopyVertsFromCPUMesh( &builder );
}


void RigidBody2D::AddColliderToList( Collider2D* colliderIn ) {
    int numColliders = (int)m_colliders.size();

    for( int colliderIndex = 0; colliderIndex < numColliders; colliderIndex++ ) {
        if( m_colliders[colliderIndex] == nullptr ) {
            m_colliders[colliderIndex] = colliderIn;
            m_area += colliderIn->GetArea();
            UpdateMomentOfInertia();
            return;
        }
    }

    m_colliders.push_back( colliderIn );
    m_area += colliderIn->GetArea();
    UpdateMomentOfInertia();
}


void RigidBody2D::AddTriggerToList( Trigger2D* triggerIn ) {
    int numTriggers = (int)m_triggers.size();

    for( int triggerIndex = 0; triggerIndex < numTriggers; triggerIndex++ ) {
        if( m_triggers[triggerIndex] == nullptr ) {
            m_triggers[triggerIndex] = triggerIn;
            return;
        }
    }

    m_triggers.push_back( triggerIn );
}


void RigidBody2D::UpdateMomentOfInertia() {
    int numColliders = (int)m_colliders.size();
    m_moment = 0.f;

    for( int colliderIndex = 0; colliderIndex < numColliders; colliderIndex++ ) {
        Collider2D* collider = m_colliders[colliderIndex];
        if( collider != nullptr ) {
            float massCollider = GetMass() * (collider->GetArea() / m_area);
            m_moment += collider->SetMomentOfInertia( massCollider );
        }
    }
}


Rgba RigidBody2D::GetAutoDebugColor() const {
    if( m_simMode == SIMULATION_MODE_STATIC ) {
        return DEBUG_COLOR_STATIC;
    } else if( m_simMode == SIMULATION_MODE_DYNAMIC ) {
        return DEBUG_COLOR_DYNAMIC;
    }

    return Rgba::WHITE;
}


Rgba RigidBody2D::GetAutoColliderColor( bool isColliding ) const {
    if( m_simMode == SIMULATION_MODE_STATIC && !isColliding ) {
        return DEBUG_COLOR_STATIC;
    } else if( m_simMode == SIMULATION_MODE_STATIC && isColliding ) {
        return DEBUG_COLOR_STATIC_COLLIDING;
    } else if( m_simMode == SIMULATION_MODE_DYNAMIC && !isColliding ) {
        return DEBUG_COLOR_DYNAMIC;
    } else if( m_simMode == SIMULATION_MODE_DYNAMIC && isColliding ) {
        return DEBUG_COLOR_DYNAMIC_COLLIDING;
    }

    return Rgba::WHITE;
}


Rgba RigidBody2D::GetAutoTriggerColor( bool isColliding ) const {
    if( m_simMode == SIMULATION_MODE_STATIC && !isColliding ) {
        return DEBUG_COLOR_TRIGGER_STATIC;
    } else if( m_simMode == SIMULATION_MODE_STATIC && isColliding ) {
        return DEBUG_COLOR_TRIGGER_STATIC_COLLIDING;
    } else if( m_simMode == SIMULATION_MODE_DYNAMIC && !isColliding ) {
        return DEBUG_COLOR_TRIGGER_DYNAMIC;
    } else if( m_simMode == SIMULATION_MODE_DYNAMIC && isColliding ) {
        return DEBUG_COLOR_TRIGGER_DYNAMIC_COLLIDING;
    }

    return Rgba::WHITE;
}


void RigidBody2D::AutoUpdateDebugColor() {
    Rgba newColor = GetAutoDebugColor();
    SetDebugColor( newColor );
}


void RigidBody2D::CompareColliders( Collider2D* myCollider, Collider2D* otherCollider, bool resolveCollision ) {
    RigidBody2D* otherRB = otherCollider->GetParent();
    CollisionInfo2D collision;

    if( myCollider->GetCollisionInfo( otherCollider, collision ) ) { // Actual collision check
        // They are colliding
        myCollider->SetIsColliding( true );
        otherCollider->SetIsColliding( true );

        // Draw Contact Point
        //g_theDebugger->DrawDebugPoint( collision.manifold.contactPos, 1.f, 0.05f, Rgba::WHITE, Rgba::WHITE, DRAW_DEPTH_MODE_ALWAYS );

        // Collision callbacks
        myCollider->FireCallbackEvent( RBCHILD_EVENT_COLLIDE, &collision );
        CollisionInfo2D inverseCollision = collision.GetInverse();
        otherCollider->FireCallbackEvent( RBCHILD_EVENT_COLLIDE, &inverseCollision );

        // Update debug color on a per collider basis
        if( m_autoUpdateColor ) {
            Rgba newColor = GetAutoColliderColor( true );
            myCollider->SetDebugColor( newColor );
        }

        if( otherRB->m_autoUpdateColor ) {
            Rgba newColor = otherRB->GetAutoColliderColor( true );
            otherCollider->SetDebugColor( newColor );
        }

        // Apply correction (move apart)
        CorrectCollisionOverlap( otherRB, collision.manifold );

        // Manage the transfer of energy
        if( resolveCollision ) {
            ResolveCollisionEnergy( otherRB, collision.manifold );
        }
    }
}


void RigidBody2D::CompareTriggerList( std::vector<Collider2D*>& colliders ) {
    int numTriggers = (int)m_triggers.size();
    int numColliders = (int)colliders.size();

    for( int triggerIndex = 0; triggerIndex < numTriggers; triggerIndex++ ) {
        Trigger2D* trigger = m_triggers[triggerIndex];

        if( (trigger != nullptr) && (!trigger->m_isGarbage) ) {
            for( int colliderIndex = 0; colliderIndex < numColliders; colliderIndex++ ) {
                Collider2D* collider = colliders[colliderIndex];

                if( (collider != nullptr) && (!collider->m_isGarbage) ) { // Both colliders are valid
                    CompareTrigger( trigger, collider );
                }
            }
        }
    }
}


void RigidBody2D::CompareTrigger( Trigger2D* trigger, Collider2D* collider ) {
    TriggerInfo2D triggerInfo;
    int currentFrame = GetPhysicsFrame();
    bool isColliding = trigger->GetTriggerInfo( collider, triggerInfo ); // Actual collision check

    if( isColliding ) {
        if( triggerInfo.firstFrame == currentFrame ) {
            trigger->FireCallbackEvent( RBCHILD_EVENT_ENTER, &triggerInfo );
            collider->m_overlappingTriggers.push_back( trigger );
        }
    } else if( (triggerInfo.latestFrame != currentFrame) && (triggerInfo.firstFrame >= 0) ) {
        // Not overlapping, but used to be
        trigger->FireCallbackEvent( RBCHILD_EVENT_LEAVE, &triggerInfo );

        int numTriggers = (int)collider->m_overlappingTriggers.size();

        for( int triggerIndex = 0; triggerIndex < numTriggers; triggerIndex++ ) {
            Trigger2D*& triggerRef = collider->m_overlappingTriggers[triggerIndex];

            if( trigger ==  triggerRef ) {
                triggerRef = nullptr;
            }
        }
    }

    // Update trigger debug color
    if( m_autoUpdateColor ) {
        Rgba newColor = GetAutoTriggerColor( isColliding );
        int numColliding = (int)trigger->m_triggerInfos.size();

        if( isColliding || (numColliding == 0 ) ) {
            // Only change if I am colliding OR no one is colliding
            trigger->SetDebugColor( newColor );
        }
    }
}


// Moves objects so they are no longer overlapping
void RigidBody2D::CorrectCollisionOverlap( RigidBody2D* otherRigidBody, const Manifold2& collision ) {
    SimulationMode otherMode = otherRigidBody->GetSimulationMode();

    if( m_simMode == SIMULATION_MODE_STATIC && otherMode == SIMULATION_MODE_STATIC ) {
        return;
    }

    float myMass = GetMass();
    float otherMass = otherRigidBody->GetMass();
    float totalMass = myMass + otherMass;

    float myCorrection = 0.f;
    float otherCorrection = 0.f;

    if( m_simMode == SIMULATION_MODE_DYNAMIC && otherMode == SIMULATION_MODE_DYNAMIC ) {
        myCorrection = otherMass / totalMass;
        otherCorrection = myMass / totalMass;
    } else if( m_simMode == SIMULATION_MODE_DYNAMIC && otherMode == SIMULATION_MODE_STATIC ) {
        // Should only ever be called when I'm dynamic and other is static, but being safe
        myCorrection = 1.f;
        otherCorrection = 0.f;
    } else if( m_simMode == SIMULATION_MODE_STATIC && otherMode == SIMULATION_MODE_DYNAMIC ) {
        myCorrection = 0.f;
        otherCorrection = 1.f;
    }

    Translate( collision.normal * collision.penetration * myCorrection );
    otherRigidBody->Translate( -1.f * collision.normal * collision.penetration * otherCorrection );
}


// Handles the energy transfer after a collision occurs
void RigidBody2D::ResolveCollisionEnergy( RigidBody2D* otherRigidBody, const Manifold2& collision ) {
    // Collision Impulse (bounce) - Only used for Frictional Coulomb's Law
    float collisionMagnitude = GetCollisionImpulseAlongNormal( otherRigidBody, collision.contactPos, collision.normal );

    if( collisionMagnitude < 0.f ) {
        return;
    }

    // Friction Impulse
    Vec2 frictionNormal = collision.normal.GetRotated90Degrees();
    float frictionScale = sqrt( abs( m_friction * otherRigidBody->m_friction ) );
    float frictionMagnitude = GetCollisionImpulseAlongNormal( otherRigidBody, collision.contactPos, frictionNormal );

    float coulumbLimit = frictionScale * collisionMagnitude;
    frictionMagnitude = Clamp( frictionMagnitude, -coulumbLimit, coulumbLimit );
    Vec2 frictionImpulse = frictionMagnitude * frictionNormal * frictionScale;

    AddImpulse( frictionImpulse, collision.contactPos );
    otherRigidBody->AddImpulse( -frictionImpulse, collision.contactPos );


    // Collision Impulse (bounce) - Actually apply normal impulse
    collisionMagnitude = GetCollisionImpulseAlongNormal( otherRigidBody, collision.contactPos, collision.normal );
    Vec2 collisionImpulse = collisionMagnitude * collision.normal;

    // DEBUGGING
    /*
    Vec2 collisionEnd = collision.contactPos + collisionImpulse;
    Vec2 frictionEnd = collision.contactPos + frictionImpulse;
    Vec2 impulseEnd = collision.contactPos + impulse;
    g_theDebugger->DrawDebugArrow( collision.contactPos, collisionEnd, 2.f, 0.1f, Rgba::RED, Rgba::RED );
    g_theDebugger->DrawDebugArrow( collision.contactPos, frictionEnd, 2.f, 0.1f, Rgba::GREEN, Rgba::GREEN );
    g_theDebugger->DrawDebugArrow( collision.contactPos, impulseEnd, 2.f, 0.1f, Rgba::BLUE, Rgba::BLUE );

    g_theDebugger->DrawDebugText( collisionEnd, Vec2::ONE, collisionImpulse.GetAsString(), 2.f );
    g_theDebugger->DrawDebugText( frictionEnd, Vec2::ONE, frictionImpulse.GetAsString(), 2.f );
    g_theDebugger->DrawDebugText( impulseEnd, Vec2::ONE, impulse.GetAsString(), 2.f );
    */
    // DEBUGGING

    AddImpulse( collisionImpulse, collision.contactPos );
    otherRigidBody->AddImpulse( -collisionImpulse, collision.contactPos );
}


float RigidBody2D::GetCollisionImpulseAlongNormal( RigidBody2D* otherRigidBody, const Vec2& contactPos, const Vec2& normal ) {
    // Impulse equations from GameDevTutorials
    // https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-oriented-rigid-bodies--gamedev-8032
    // http://chrishecker.com/images/e/e7/Gdmphys3.pdf

    Vec2 vDiffInitial = GetImpactVelocity( contactPos ) - otherRigidBody->GetImpactVelocity( contactPos );
    float magnitudeNormal = DotProduct( vDiffInitial, normal );

    float invMassA = 1.f / GetMass();
    float invMassB = 1.f / otherRigidBody->GetMass();

    Vec2 centerToContactA = contactPos - m_transform.position;
    Vec2 centerToContactB = contactPos - otherRigidBody->m_transform.position;

    float contactMagA = CrossProductLength( centerToContactA, normal );
    float contactMagB = CrossProductLength( centerToContactB, normal );

    float rotationalA = contactMagA * contactMagA / GetMomentOfInertia();
    float rotationalB = contactMagB * contactMagB / otherRigidBody->GetMomentOfInertia();

    // For Static v Dynamic, invMass and rotational of the static object will go to zero;
    float restitutionScale = Min( m_restitution, otherRigidBody->m_restitution );
    float impulseMagnitude = -(restitutionScale + 1) * magnitudeNormal / (invMassA + invMassB + rotationalA + rotationalB);

    return impulseMagnitude;
}
