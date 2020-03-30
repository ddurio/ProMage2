#include "Engine/Physics/PhysicsSystem.hpp"

#include "Engine/Renderer/RenderContext.hpp"


const float PhysicsSystem::s_fixedStepSeconds = 0.05f;


PhysicsSystem::PhysicsSystem( RenderContext* renderContext ) :
    m_renderContext(renderContext) {
}


void PhysicsSystem::Startup() {

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

}


void PhysicsSystem::BeginFrame() {
    std::scoped_lock localLock( m_mutex );

    std::vector<RigidBody2D*>& rbList = m_rigidBodies[SIMULATION_MODE_DYNAMIC];
    int numRigidBodies = (int)rbList.size();

    for( int rigidBodyIndex = 0; rigidBodyIndex < numRigidBodies; rigidBodyIndex++ ) {
        RigidBody2D* rigidBody = rbList[rigidBodyIndex];

        if( rigidBody != nullptr ) {
            rigidBody->BeginFrame(); // Clears frame forces and torques
        }
    }
}


void PhysicsSystem::EndFrame() {
    std::scoped_lock localLock( m_mutex );

    CollectGarbage(); // Special case for when no update steps are run (i.e. thesis)
}


void PhysicsSystem::Update( float deltaSeconds ) {
    for( int simModeIndex = 0; simModeIndex < NUM_SIMULATION_MODES; simModeIndex++ ) {
        std::vector<RigidBody2D*>& rbList = m_rigidBodies[simModeIndex];
        int numRigidBodies = (int)rbList.size();

        for( int rigidbodyIndex = 0; rigidbodyIndex < numRigidBodies; rigidbodyIndex++ ) {
            RigidBody2D* rigidBody = rbList[rigidbodyIndex];

            if( rigidBody != nullptr ) {
                rigidBody->ClearCollision();
            }
        }
    }

    
    // Fixed Step Physics Updates
    float remainingSeconds = deltaSeconds;

    while( remainingSeconds > 0.f ) {
        float stepSeconds = (remainingSeconds > s_fixedStepSeconds) ? s_fixedStepSeconds : remainingSeconds;
        RunUpdateStep( stepSeconds );
        remainingSeconds -= stepSeconds;
    }

    UpdateGameObjectTransforms();
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


void PhysicsSystem::RunUpdateStep( float deltaSeconds ) {
    m_stepClock->Tick( deltaSeconds );

    std::vector<RigidBody2D*>& staticList = m_rigidBodies[SIMULATION_MODE_STATIC];
    std::vector<RigidBody2D*>& dynamicList = m_rigidBodies[SIMULATION_MODE_DYNAMIC];

    // Update all DYNAMIC rigid bodies only
    UpdateRigidBodyList( deltaSeconds, dynamicList );

    // Update collision
    UpdateCollisionBetweenLists( dynamicList, staticList, true );
    UpdateCollisionBetweenLists( dynamicList, dynamicList, true );
    UpdateCollisionBetweenLists( dynamicList, staticList, false );

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


void PhysicsSystem::UpdateGameObjectTransforms() {
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