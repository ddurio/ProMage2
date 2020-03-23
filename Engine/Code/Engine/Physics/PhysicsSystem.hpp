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

    // Physics Channels / Filters
    int AddChannel( const std::string& channelName );
    void EnableChannelCollision( const std::string& channelName1, const std::string& channelName2 );
    void DisableChannelCollision( const std::string& channelName1, const std::string& channelName2 );
    void EnableAllCollision( const std::string& channelName );
    void DisableAllCollision( const std::string& channelName );

    int GetChannelIndex( const std::string& channelName ) const;
    bool DoChannelsCollide( const std::string& channelName1, const std::string& channelName2 ) const;
    bool DoChannelsCollide( int channelIndex1, int channelIndex2 ) const;

    // Bit Fields / Regions
    void SetBitFieldBounds( const AABB2& worldBounds );

    bool IsUsingBitFields() const;
    Vec2 GetBitFieldScale() const;
    Vec2 GetBitFieldScaledOffset() const;

    const Vec2& GetGravity() const;
    RenderContext* GetRenderContext() const;
    int GetPhysicsFrame() const;

    void SetGravity( const Vec2& gravity );
    void SetGravity( float gravityX, float gravityY );
    void UpdateRigidBodyMode( RigidBody2D* rigidBody, SimulationMode oldMode, SimulationMode newMode );


    static DevConsoleChannel s_physConsoleChannel;


    private:
    static const int MAX_CHANNELS = 32;
    static const int COLLISION_MATRIX_SIZE = MAX_CHANNELS * MAX_CHANNELS;

    struct PhysicsChannel {
        std::string name = "";
        int index = 0;
    };


    RenderContext* m_renderContext;
    Clock* m_stepClock = new Clock( nullptr ); // Purely tracks "frame" numbers (actually number of physics steps)

    Vec2 m_gravity = Vec2::ZERO;
    std::vector<RigidBody2D*> m_rigidBodies[NUM_SIMULATION_MODES + 1]; // Store RBs for deletion at last slot of array (so num plus one)

    std::recursive_mutex m_mutex;
    float m_prevRemainderSeconds = 0.f;
    static constexpr float s_fixedStepSeconds = 0.01f;

    // Channels
    std::map< std::string, PhysicsChannel, StringCmpCaseI > m_channelDetails;
    bool m_collisionMatrix[COLLISION_MATRIX_SIZE] = {};
    int m_numChannels = 0;

    // BitFields
    bool m_isUsingBitFields = false;
    Vec2 m_bitFieldScale = Vec2::ZERO;
    Vec2 m_bitFieldOffset = Vec2::ZERO;


    void RunFixedUpdateStep();
    void UpdateRigidBodyList( float deltaSeconds, std::vector<RigidBody2D*>& rbList );
    void UpdateCollisionBetweenLists( const std::vector<RigidBody2D*>& rbList1, const std::vector<RigidBody2D*>& rbList2, bool resolveCollision );
    void UpdateTriggersBetweenLists( const std::vector<RigidBody2D*>& rbList1, const std::vector<RigidBody2D*>& rbList2 );
    void CopyGameObjToPhysicsObj();
    void CopyPhysicsObjToGameObj();
    void CollectGarbage();

    void AddRigidBodyToList( RigidBody2D* rigidBodyIn, SimulationMode simMode );
    void RemoveRigidBodyFromList( RigidBody2D* rigidBodyIn, SimulationMode simMode );

    IntVec2 GetCollisionMatrixIndices( int channelIndex1, int channelIndex2 ) const;
    IntVec2 GetCollisionMatrixIndices( const std::string& channelName1, const std::string& channelName2 ) const;
};
