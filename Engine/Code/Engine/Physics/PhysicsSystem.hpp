#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Physics/RigidBody2D.hpp"

#include "mutex"


class PhysicsSystem {
    public:
    PhysicsSystem( RenderContext* renderContext );

    void Startup();
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void Update( float deltaSeconds );
    void RenderDebug() const;

    RigidBody2D* CreateNewRigidBody( float mass, const Rgba& debugColor = Rgba::WHITE );
    void DestroyRigidBody( RigidBody2D* rigidbody );

    const Vec2& GetGravity() const;
    RenderContext* GetRenderContext() const;
    int GetPhysicsFrame() const;

    void SetGravity( const Vec2& gravity );
    void SetGravity( float gravityX, float gravityY );
    void UpdateRigidBodyMode( RigidBody2D* rigidBody, SimulationMode oldMode, SimulationMode newMode );

    private:
    RenderContext* m_renderContext;
    Clock* m_stepClock = new Clock( nullptr ); // Purely tracks "frame" numbers (actually number of physics steps)

    Vec2 m_gravity = Vec2::ZERO;
    std::vector<RigidBody2D*> m_rigidBodies[NUM_SIMULATION_MODES + 1]; // Store RBs for deletion at last slot of array (so num plus one)

    std::recursive_mutex m_mutex;

    static const float s_fixedStepSeconds;


    void RunUpdateStep( float deltaSeconds );
    void UpdateRigidBodyList( float deltaSeconds, std::vector<RigidBody2D*>& rbList );
    void UpdateCollisionBetweenLists( const std::vector<RigidBody2D*>& rbList1, const std::vector<RigidBody2D*>& rbList2, bool resolveCollision );
    void UpdateTriggersBetweenLists( const std::vector<RigidBody2D*>& rbList1, const std::vector<RigidBody2D*>& rbList2 );
    void UpdateGameObjectTransforms();
    void CollectGarbage();

    void AddRigidBodyToList( RigidBody2D* rigidBodyIn, SimulationMode simMode );
    void RemoveRigidBodyFromList( RigidBody2D* rigidBodyIn, SimulationMode simMode );
};
