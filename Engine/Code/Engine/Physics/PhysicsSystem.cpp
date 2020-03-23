#include "Engine/Physics/PhysicsSystem.hpp"

#include "Engine/Debug/Profiler.hpp"
#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"


DevConsoleChannel PhysicsSystem::s_physConsoleChannel = DevConsole::CHANNEL_UNDEFINED; // PUBLIC


PhysicsSystem::PhysicsSystem( RenderContext* renderContext ) :
    m_renderContext(renderContext) {
    g_theDevConsole->PrintString( "(PhysicsSystem) Initializing...", s_physConsoleChannel );

    if( s_physConsoleChannel == DevConsole::CHANNEL_UNDEFINED ) {
        s_physConsoleChannel = g_theDevConsole->AddChannel( "Physics", Rgba::ORGANIC_PURPLE );
    }
}


void PhysicsSystem::Startup() {
    // All channels default to colliding
    for( int channelIndex = 0; channelIndex < COLLISION_MATRIX_SIZE; channelIndex++ ) {
        m_collisionMatrix[channelIndex] = true;
    }

    // Create first channel
    AddChannel( "Default" );
}


void PhysicsSystem::Shutdown() {
    for( int simModeIndex = 0; simModeIndex < NUM_SIMULATION_MODES; simModeIndex++ ) {
        std::vector<RigidBody2D*>& rbList = m_rigidBodies[simModeIndex];
        int numRBs = (int)rbList.size();

        for( int rbIndex = 0; rbIndex < numRBs; rbIndex++ ) {
            delete rbList[rbIndex];
            rbList[rbIndex] = nullptr;
        }

        rbList.clear();
    }

    g_theDevConsole->PrintString( "(PhysicsSystem) Shutdown complete", s_physConsoleChannel );
}


void PhysicsSystem::BeginFrame() {
    PROFILE_FUNCTION();

    std::scoped_lock localLock( m_mutex );

    for( int simModeIndex = 0; simModeIndex < NUM_SIMULATION_MODES; simModeIndex++ ) {
        std::vector<RigidBody2D*>& rbList = m_rigidBodies[simModeIndex];
        int numRigidBodies = (int)rbList.size();

        for( int rigidBodyIndex = 0; rigidBodyIndex < numRigidBodies; rigidBodyIndex++ ) {
            RigidBody2D* rigidBody = rbList[rigidBodyIndex];

            if( rigidBody != nullptr ) {
                rigidBody->BeginFrame(); // Clears frame forces and torques
            }
        }
    }
}


void PhysicsSystem::EndFrame() {
    std::scoped_lock localLock( m_mutex );

    CollectGarbage(); // Special case for when no update steps are run (i.e. thesis)
}


void PhysicsSystem::Update( float deltaSeconds ) {
    PROFILE_FUNCTION();

    // Fixed Step Physics Updates
    m_prevRemainderSeconds += deltaSeconds;
    CopyGameObjToPhysicsObj();

    while( m_prevRemainderSeconds >= s_fixedStepSeconds ) {
        RunFixedUpdateStep();
        m_prevRemainderSeconds -= s_fixedStepSeconds;
    }

    CopyPhysicsObjToGameObj();
}


void PhysicsSystem::RenderDebug() const {
    m_renderContext->BindShader( nullptr );

    for( int simModeIndex = 0; simModeIndex < NUM_SIMULATION_MODES; simModeIndex++ ) {
        const std::vector<RigidBody2D*>& rbList = m_rigidBodies[simModeIndex];
        int numRigidBodies = (int)rbList.size();

        for( int rigidbodyIndex = 0; rigidbodyIndex < numRigidBodies; rigidbodyIndex++ ) {
            RigidBody2D* rigidBody = rbList[rigidbodyIndex];

            if( rigidBody != nullptr ) {
                rigidBody->RenderDebug();
            }
        }
    }
}


RigidBody2D* PhysicsSystem::CreateNewRigidBody( float mass, const Rgba& debugColor /*= Rgba::WHITE */ ) {
    RigidBody2D* rigidBody = new RigidBody2D( this, mass, debugColor );
    AddRigidBodyToList( rigidBody, rigidBody->GetSimulationMode() );
    return rigidBody;
}


void PhysicsSystem::DestroyRigidBody( RigidBody2D* rigidbody ) {
    if( rigidbody == nullptr ) {
        return;
    }

    // Can't actually remove it yet.. needs to be done in end step
    rigidbody->Destroy();
    AddRigidBodyToList( rigidbody, SIMULATION_MODE_INVALID );
}


int PhysicsSystem::AddChannel( const std::string& channelName ) {
    if( m_numChannels >= MAX_CHANNELS ) {
        std::string warningMsg = Stringf( "(PhysicsSystem) WARNING -- Max number of PhysicsChannels reached! Failed to add channel (%s)", channelName.c_str() );
        g_theDevConsole->PrintString( warningMsg, DevConsole::CHANNEL_WARNING | s_physConsoleChannel );
        return 0; // Return default.. collides with everything
    }

    // Add to registry
    PhysicsChannel& newChannel = m_channelDetails[channelName];

    if( StringICmp( newChannel.name, channelName ) ) { // Already been created
        std::string warningMsg = Stringf( "(PhysicsSystem) WARNING -- Channel (%s) already exists!", channelName.c_str() );
        g_theDevConsole->PrintString( warningMsg, DevConsole::CHANNEL_WARNING | s_physConsoleChannel );
    } else if( StringICmp( channelName, "Default" ) && m_numChannels != 0 ) { // Default is not index 0
        std::string errorMsg = "(PhysicsSystem) ERROR -- Channel named \"Default\" was not the first channel created.  Did you forget to call Startup?";
        g_theDevConsole->PrintString( errorMsg, DevConsole::CHANNEL_ERROR | s_physConsoleChannel );
    } else {
        // Set values
        newChannel.name = channelName;
        newChannel.index = m_numChannels;

        m_numChannels++;
    }

    // Return channel index
    return newChannel.index;
}


void PhysicsSystem::EnableChannelCollision( const std::string& channelName1, const std::string& channelName2 ) {
    IntVec2 matrixIndices = GetCollisionMatrixIndices( channelName1, channelName2 );

    if( matrixIndices.x != -1 ) {
        m_collisionMatrix[matrixIndices.x] = true;
    }

    if( matrixIndices.y != -1 ) {
        m_collisionMatrix[matrixIndices.y] = true;
    }
}


void PhysicsSystem::DisableChannelCollision( const std::string& channelName1, const std::string& channelName2 ) {
    if( StringICmp( channelName1, "Default" ) ||
        StringICmp( channelName2, "Default" ) ) {
        return; // Cannot disable "Default" channel
    }

    IntVec2 matrixIndices = GetCollisionMatrixIndices( channelName1, channelName2 );

    if( matrixIndices.x != -1 ) {
        m_collisionMatrix[matrixIndices.x] = false;
    }

    if( matrixIndices.y != -1 ) {
        m_collisionMatrix[matrixIndices.y] = false;
    }
}


void PhysicsSystem::EnableAllCollision( const std::string& channelName ) {
    int channelIndex = GetChannelIndex( channelName );

    if( channelIndex <= 0) {
        return; // Cannot disable "Default" channel (index == 0)
    }

    for( int otherChannelIndex = 1; otherChannelIndex < MAX_CHANNELS; otherChannelIndex++ ) {
        // Note: starting at 1.. "Default" channel at index 0 always collides with everything
        IntVec2 matrixIndices = GetCollisionMatrixIndices( channelIndex, otherChannelIndex );

        if( matrixIndices != IntVec2::NEGONE ) {
            m_collisionMatrix[matrixIndices.x] = true;
            m_collisionMatrix[matrixIndices.y] = true;
        }
    }
}


void PhysicsSystem::DisableAllCollision( const std::string& channelName ) {
    int channelIndex = GetChannelIndex( channelName );

    if( channelIndex <= 0) {
        return; // Cannot disable "Default" channel (index == 0)
    }

    for( int otherChannelIndex = 1; otherChannelIndex < MAX_CHANNELS; otherChannelIndex++ ) {
        // Note: starting at 1.. "Default" channel at index 0 always collides with everything
        IntVec2 matrixIndices = GetCollisionMatrixIndices( channelIndex, otherChannelIndex );

        if( matrixIndices != IntVec2::NEGONE ) {
            m_collisionMatrix[matrixIndices.x] = false;
            m_collisionMatrix[matrixIndices.y] = false;
        }
    }
}


int PhysicsSystem::GetChannelIndex( const std::string& channelName ) const {
    std::map< std::string, PhysicsChannel, StringCmpCaseI >::const_iterator channelIter = m_channelDetails.find( channelName );

    if( channelIter != m_channelDetails.end() ) {
        return channelIter->second.index;
    }

    return -1;
}


bool PhysicsSystem::DoChannelsCollide( const std::string& channelName1, const std::string& channelName2 ) const {
    IntVec2 matrixIndices = GetCollisionMatrixIndices( channelName1, channelName2 );

    if( matrixIndices == IntVec2::NEGONE ) {
        return true; // Not sure.. but default to yes
    }

    return m_collisionMatrix[matrixIndices.x]; // X or Y should (in theory) return the same value
}


bool PhysicsSystem::DoChannelsCollide( int channelIndex1, int channelIndex2 ) const {
    IntVec2 matrixIndices = GetCollisionMatrixIndices( channelIndex1, channelIndex2 );

    if( matrixIndices == IntVec2::NEGONE ) {
        return true; // Not sure.. but default to yes
    }

    return m_collisionMatrix[matrixIndices.x]; // X or Y should (in theory) return the same value
}


void PhysicsSystem::SetBitFieldBounds( const AABB2& worldBounds ) {
    m_isUsingBitFields = true;

    m_bitFieldScale = Vec2( 8.f ) / worldBounds.GetDimensions();
    m_bitFieldOffset = -worldBounds.mins * m_bitFieldScale; // Technically (origin - mins) * scale
}


bool PhysicsSystem::IsUsingBitFields() const {
    return m_isUsingBitFields;
}


Vec2 PhysicsSystem::GetBitFieldScale() const {
    return m_bitFieldScale;
}


Vec2 PhysicsSystem::GetBitFieldScaledOffset() const {
    return m_bitFieldOffset;
}


const Vec2& PhysicsSystem::GetGravity() const {
    return m_gravity;
}


RenderContext* PhysicsSystem::GetRenderContext() const {
    return m_renderContext;
}


int PhysicsSystem::GetPhysicsFrame() const {
    return m_stepClock->GetFrameNumber();
}


void PhysicsSystem::SetGravity( const Vec2& gravity ) {
    m_gravity = gravity;
}


void PhysicsSystem::SetGravity( float gravityX, float gravityY ) {
    Vec2 gravityXY = Vec2( gravityX, gravityY );
    SetGravity( gravityXY );
}


void PhysicsSystem::UpdateRigidBodyMode( RigidBody2D* rigidBody, SimulationMode oldMode, SimulationMode newMode ) {
    if( rigidBody->GetSimulationMode() != newMode ) {
        // Safety net in case game calls this by mistake
        rigidBody->SetSimulationMode( newMode ); // this will call me back under the hood
        return;
    }

    RemoveRigidBodyFromList( rigidBody, oldMode );
    AddRigidBodyToList( rigidBody, newMode );
}


// PRIVATE ------------------------------------------------------------------------------------------
void PhysicsSystem::RunFixedUpdateStep() {
    PROFILE_FUNCTION();
    m_stepClock->Tick( s_fixedStepSeconds );

    std::vector<RigidBody2D*>& staticList = m_rigidBodies[SIMULATION_MODE_STATIC];
    std::vector<RigidBody2D*>& dynamicList = m_rigidBodies[SIMULATION_MODE_DYNAMIC];

    // Update all DYNAMIC rigid bodies only
    UpdateRigidBodyList( s_fixedStepSeconds, dynamicList );

    // Update collision
    UpdateCollisionBetweenLists( dynamicList, dynamicList, true );
    UpdateCollisionBetweenLists( dynamicList, staticList, true );


    // Update triggers
    UpdateTriggersBetweenLists( dynamicList, dynamicList );
    UpdateTriggersBetweenLists( dynamicList, staticList );

    // Actually destroy rigidBodies
    CollectGarbage();
}


void PhysicsSystem::UpdateRigidBodyList( float deltaSeconds, std::vector<RigidBody2D*>& rbList ) {
    int numRigidBodies = (int)rbList.size();

    for( int rigidBodyIndex = 0; rigidBodyIndex < numRigidBodies; rigidBodyIndex++ ) {
        RigidBody2D* rigidBody = rbList[rigidBodyIndex];

        if( rigidBody != nullptr ) {
            rigidBody->Update( deltaSeconds );
        }
    }
}


void PhysicsSystem::UpdateCollisionBetweenLists( const std::vector<RigidBody2D*>& rbList1, const std::vector<RigidBody2D*>& rbList2, bool resolveCollision ) {
    // FIXME: resolveCollision option is essentially deprecated.. should always be true, maybe leave?
    PROFILE_FUNCTION();
    bool listsAreTheSame = (&rbList1 == &rbList2);

    RigidBody2D* rigidBody1 = nullptr;
    RigidBody2D* rigidBody2 = nullptr;

    int numRigidBodies1 = (int)rbList1.size();
    int numRigidBodies2 = (int)rbList2.size();

    for( int rigidBody1Index = 0; rigidBody1Index < numRigidBodies1; rigidBody1Index++ ) {
        rigidBody1 = rbList1[rigidBody1Index];

        if( (rigidBody1 != nullptr) && (!rigidBody1->m_isGarbage) ) {
            int rigidBody2Index = listsAreTheSame ? (rigidBody1Index + 1) : 0;

            for( rigidBody2Index; rigidBody2Index < numRigidBodies2; rigidBody2Index++ ) {
                rigidBody2 = rbList2[rigidBody2Index];

                if( (rigidBody2 != nullptr) && (!rigidBody2->m_isGarbage) ) {
                    rigidBody1->CheckCollision( rigidBody2, resolveCollision );
                }
            }
        }
    }
}


void PhysicsSystem::UpdateTriggersBetweenLists( const std::vector<RigidBody2D*>& rbList1, const std::vector<RigidBody2D*>& rbList2 ) {
    bool listsAreTheSame = (&rbList1 == &rbList2);

    RigidBody2D* rigidBody1 = nullptr;
    RigidBody2D* rigidBody2 = nullptr;

    int numRigidBodies1 = (int)rbList1.size();
    int numRigidBodies2 = (int)rbList2.size();

    for( int rigidBody1Index = 0; rigidBody1Index < numRigidBodies1; rigidBody1Index++ ) {
        rigidBody1 = rbList1[rigidBody1Index];

        if( (rigidBody1 != nullptr) && (!rigidBody1->m_isGarbage) ) {
            int rigidBody2Index = listsAreTheSame ? (rigidBody1Index + 1) : 0;

            for( rigidBody2Index; rigidBody2Index < numRigidBodies2; rigidBody2Index++ ) {
                rigidBody2 = rbList2[rigidBody2Index];

                if( (rigidBody2 != nullptr) && (!rigidBody2->m_isGarbage) ) {
                    rigidBody1->CheckTriggers( rigidBody2 );
                }
            }
        }
    }
}


void PhysicsSystem::CopyGameObjToPhysicsObj() {
    std::vector<RigidBody2D*>& rbList = m_rigidBodies[SIMULATION_MODE_DYNAMIC];
    int numRigidBodies = (int)rbList.size();

    for( int rbIndex = 0; rbIndex < numRigidBodies; rbIndex++ ) {
        RigidBody2D* rigidBody = rbList[rbIndex];

        if( rigidBody != nullptr ) {
            rigidBody->UpdatePosition();
        }
    }
}


void PhysicsSystem::CopyPhysicsObjToGameObj() {
    std::vector<RigidBody2D*>& rbList = m_rigidBodies[SIMULATION_MODE_DYNAMIC];
    int numRigidBodies = (int)rbList.size();

    for( int rbIndex = 0; rbIndex < numRigidBodies; rbIndex++ ) {
        RigidBody2D* rigidBody = rbList[rbIndex];

        if( rigidBody != nullptr ) {
            rigidBody->UpdateGameObjectTransform();
        }
    }
}


void PhysicsSystem::CollectGarbage() {
    std::vector<RigidBody2D*>& rbList = m_rigidBodies[NUM_SIMULATION_MODES];
    int numRigidBodies = (int)rbList.size();

    for( int rbIndex = 0; rbIndex < numRigidBodies; rbIndex++ ) {
        RigidBody2D*& rigidBody = rbList[rbIndex];
        RemoveRigidBodyFromList( rigidBody, rigidBody->GetSimulationMode() );
        CLEAR_POINTER( rigidBody );
    }

    rbList.clear();
}


void PhysicsSystem::AddRigidBodyToList( RigidBody2D* rigidBodyIn, SimulationMode simMode ) {
    std::scoped_lock localLock( m_mutex );

    if( simMode == SIMULATION_MODE_INVALID ) {
        // Store RBs for deletion at last slot of array
        simMode = NUM_SIMULATION_MODES;
    }

    std::vector<RigidBody2D*>& rbList = m_rigidBodies[simMode];
    int numRigidBodies = (int)rbList.size();

    for( int rigidbodyIndex = 0; rigidbodyIndex < numRigidBodies; rigidbodyIndex++ ) {
        if( rbList[rigidbodyIndex] == nullptr ) {
            rbList[rigidbodyIndex] = rigidBodyIn;
            return;
        }
    }

    rbList.push_back( rigidBodyIn );
}


void PhysicsSystem::RemoveRigidBodyFromList( RigidBody2D* rigidBodyIn, SimulationMode simMode ) {
    std::scoped_lock localLock( m_mutex );

    std::vector<RigidBody2D*>& rbList = m_rigidBodies[simMode];
    int numRigidBodies = (int)rbList.size();

    for( int rigidbodyIndex = 0; rigidbodyIndex < numRigidBodies; rigidbodyIndex++ ) {
        if( rbList[rigidbodyIndex] == rigidBodyIn ) {
            rbList[rigidbodyIndex] = nullptr;
            return;
        }
    }
}


IntVec2 PhysicsSystem::GetCollisionMatrixIndices( int channelIndex1, int channelIndex2 ) const {
    if( channelIndex1 < 0 || channelIndex1 >= MAX_CHANNELS ||
        channelIndex2 < 0 || channelIndex2 >= MAX_CHANNELS ) {
        return IntVec2::NEGONE;
    }

    int matrixIndex1 = (channelIndex1 * MAX_CHANNELS) + channelIndex2;
    int matrixIndex2 = (channelIndex2 * MAX_CHANNELS) + channelIndex1;

    return IntVec2( matrixIndex1, matrixIndex2 );
}


IntVec2 PhysicsSystem::GetCollisionMatrixIndices( const std::string& channelName1, const std::string& channelName2 ) const {
    int channelIndex1 = GetChannelIndex( channelName1 );
    int channelIndex2 = GetChannelIndex( channelName2 );

    return GetCollisionMatrixIndices( channelIndex1, channelIndex2 );
}
