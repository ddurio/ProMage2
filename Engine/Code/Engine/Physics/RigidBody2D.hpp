#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Transform2D.hpp"


class Collider2D;
class GPUMesh;
class PhysicsSystem;
class RenderContext;
class Trigger2D;


struct AABB2;
struct Capsule2;
struct Manifold2;
struct OBB2;


enum SimulationMode {
    SIMULATION_MODE_INVALID = -1,

    SIMULATION_MODE_STATIC,
    SIMULATION_MODE_DYNAMIC,

    NUM_SIMULATION_MODES
};


const std::string PHYSICS_ARG_MY_COLLIDER       = "myCollider";
const std::string PHYSICS_ARG_MY_TRIGGER        = "myTrigger";
const std::string PHYSICS_ARG_OTHER_COLLIDER    = "otherCollider";
const std::string PHYSICS_ARG_MANIFOLD          = "manifold";
const std::string PHYSICS_ARG_FIRST_FRAME       = "firstFrame";
const std::string PHYSICS_ARG_LATEST_FRAME      = "latestFrame";


class RigidBody2D {
    friend class PhysicsSystem;

    public:
    void BeginFrame();

    void Update( float deltaSeconds );
    void UpdatePosition();
    void UpdateBitFields();

    void ClearCollisionDebug();
    void CheckCollision( RigidBody2D* otherRB, bool resolveCollision );
    void CheckTriggers( RigidBody2D* otherRB );
    void UpdateGameObjectTransform();
    void RenderDebug() const;

    SimulationMode GetSimulationMode() const;
    int GetChannelIndex() const;
    Rgba GetDebugColor() const;
    float GetMass() const;
    float GetRestitution() const;
    float GetFriction() const;
    float GetMomentOfInertia() const;
    float GetLinearDrag() const;
    float GetAngularDrag() const;
    Vec2 GetVelocity() const;
    Vec2 GetImpactVelocity( const Vec2& worldPos ) const;
    float GetAngularVelocity() const;
    bool GetXConstraint() const;
    bool GetYConstraint() const;
    bool GetRotationConstraint() const;
    const std::vector<Collider2D*>& GetColliders() const;
    const Transform2D& GetTransform() const;
    int GetPhysicsFrame() const;

    Collider2D* AddCollider( const AABB2& localBounds, float radius = 0 );
    Collider2D* AddCollider( const OBB2& localBounds, float radius = 0 );
    Collider2D* AddCollider( const Capsule2& localBounds );
    Collider2D* AddCollider( const Vec2& discLocalCenter, float discRadius );

    Trigger2D* AddTrigger( const AABB2& localBounds, float radius = 0 );
    Trigger2D* AddTrigger( const OBB2& localBounds, float radius = 0 );
    Trigger2D* AddTrigger( const Capsule2& localBounds );
    Trigger2D* AddTrigger( const Vec2& discLocalCenter, float discRadius );

    void DestroyCollider( Collider2D* collider );

    void AddForce( const Vec2& newForce );
    void AddTorque( float torqueDegrees );
    void AddImpulse( const Vec2& impulse, const Vec2& contactPos, bool ignoreMass = false );

    void SetSimulationMode( SimulationMode mode );
    void SetGameObjectTransform( Transform2D* gameObjectTransform );
    void SetIgnoreObject( RigidBody2D* ignoreObject );
    void SetChannel( const std::string& channelName );
    void SetDebugColor( const Rgba& color );
    void SetAutoDebugColor( bool autoUpdateColor );
    void SetGravityScale( const Vec2& gravityScale );
    void SetMass( float mass );
    void SetRestitution( float restitution );
    void SetFriction( float friction );
    void SetLinearDrag( float linearDrag );
    void SetAngularDrag( float angularDrag );
    void SetConstraints( bool xIsFixed, bool yIsFixed, bool rotIsFixed );

    void Translate( const Vec2& translation );
    void UpdateTriggerColor( Trigger2D* trigger );

    static const Rgba& DEBUG_COLOR_STATIC;
    static const Rgba& DEBUG_COLOR_STATIC_COLLIDING;
    static const Rgba& DEBUG_COLOR_DYNAMIC;
    static const Rgba& DEBUG_COLOR_DYNAMIC_COLLIDING;

    static const Rgba& DEBUG_COLOR_TRIGGER_STATIC;
    static const Rgba& DEBUG_COLOR_TRIGGER_STATIC_COLLIDING;
    static const Rgba& DEBUG_COLOR_TRIGGER_DYNAMIC;
    static const Rgba& DEBUG_COLOR_TRIGGER_DYNAMIC_COLLIDING;

    private:
    RigidBody2D( PhysicsSystem* physicsSystem, float mass, const Rgba& debugColor = Rgba::WHITE );
    ~RigidBody2D();

    void Destroy();


    PhysicsSystem* m_physicsSystem = nullptr;
    RenderContext* m_renderContext = nullptr;
    std::vector<Collider2D*> m_colliders;
    std::vector<Trigger2D*> m_triggers;

    Transform2D m_transform = Transform2D();
    Transform2D* m_gameObjectTransform = nullptr;
    RigidBody2D* m_ignoreObject = nullptr;

    SimulationMode m_simMode = SIMULATION_MODE_STATIC;
    int m_channelIndex = 0; // "Default" channel is index zero
    bool m_isGarbage = false;

    Vec2 m_velocity = Vec2::ZERO;
    float m_angularVelocity = 0.f;

    Vec2 m_frameForces = Vec2::ZERO;
    float m_frameTorque = 0.f;

    Vec2 m_gravityScale = Vec2::ONE;
    float m_mass = 1.f;
    float m_restitution = 1.f;
    float m_friction = 1.f;
    float m_area = 0.f;
    float m_moment = 0.f;
    float m_linearDrag = 0.1f;
    float m_angularDrag = 0.1f;

    bool m_xIsFixed = false;
    bool m_yIsFixed = false;
    bool m_rotIsFixed = false;

    Rgba m_debugColor = Rgba::WHITE;
    //bool m_autoUpdateColor = true;
    bool m_autoUpdateColor = false;

    GPUMesh* m_mesh = nullptr;
    uint64_t m_bitFields = 0xFFFF'FFFF'FFFF'FFFF;


    void BuildMesh();
    void AddColliderToList( Collider2D* colliderIn );
    void AddTriggerToList( Trigger2D* triggerIn );
    void UpdateMomentOfInertia();

    Rgba GetAutoDebugColor() const;
    Rgba GetAutoColliderColor( bool isColliding ) const;
    Rgba GetAutoTriggerColor( bool isColliding ) const;
    void AutoUpdateDebugColor();

    bool IsImpossibleToOverlap( const RigidBody2D* otherRB ) const;
    void CompareColliders( Collider2D* myCollider, Collider2D* otherCollider, bool resolveCollision );
    void CompareTriggerList( std::vector<Collider2D*>& colliders, bool isImpossibleToOverlap );
    void CompareTrigger( Trigger2D* myTrigger, Collider2D* otherCollider, bool isImpossibleToOverlap );
    void CorrectCollisionOverlap( RigidBody2D* otherRigidBody, const Manifold2& collision );
    void ResolveCollisionEnergy( RigidBody2D* otherRigidBody, const Manifold2& collision );
    float GetCollisionImpulseAlongNormal( RigidBody2D* otherRigidBody, const Vec2& contactPos, const Vec2& normal );
};