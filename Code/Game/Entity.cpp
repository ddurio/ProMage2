#include "Game/Entity.hpp"

#include "Engine/Core/Timer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/Capsule3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/StatsManager.hpp"


class RigidBody2D;


Entity::Entity( Map* theMap, EntityType type /*= ENTITY_TYPE_UNKNOWN*/, FactionID faction /*= FACTION_UNKNOWN */ ) :
    m_map( theMap ),
    m_entityType( type ),
    m_entityIndex( s_numEntities++ ) {
    SetFaction( faction );
}


Entity::~Entity() {
    CLEAR_POINTER( m_mesh );
    CLEAR_POINTER( m_deathTimer );

    g_thePhysicsSystem->DestroyRigidBody( m_rigidBody );
}


const Vec2 Entity::GetPosition() const {
    return m_transform.position;
}


const Transform2D* Entity::GetTransformPointer() const {
    return &m_transform;
}


const FactionID Entity::GetFaction() const {
    return m_faction;
}


const EntityType Entity::GetEntityType() const {
    return m_entityType;
}


RigidBody2D* Entity::GetRigidBody() const {
    return m_rigidBody;
}


bool Entity::IsAlive() const {
    return !m_isDead;
}


bool Entity::IsGarbage() const {
    return m_isGarbage;
}


bool Entity::IsKillable() const {
    return m_canDie;
}


bool Entity::IsMovable() const {
    return m_canWalk || m_canFly || m_canSwim;
}


float Entity::GetHealth() const {
    if( m_statsManager != nullptr ) {
        return m_statsManager->GetHealth();
    }

    return 1.f;
}


float Entity::GetMaxHealth() const {
    if( m_statsManager != nullptr ) {
        return m_statsManager->GetMaxHealth();
    }

    return 1.f;
}


float Entity::GetPercentHealth() const {
    if( m_statsManager != nullptr ) {
        return m_statsManager->GetPercentHealth();
    }

    return 1.f;
}

void Entity::SetFaction( FactionID faction ) {
    m_faction = faction;

    switch( m_faction ) {
        // Intentional fallthrough for all player faction entities
        // All using the same audio clip
        case(FACTION_PLAYER0): {
        } case(FACTION_PLAYER1): {
        } case(FACTION_PLAYER2): {
        } case(FACTION_PLAYER3): {
            //m_hitSound = g_theAudio->CreateOrGetSound( AUDIO_PLAYERTANK_HIT );
            break;
        } case(FACTION_ENEMY1): {
            //m_hitSound = g_theAudio->CreateOrGetSound( AUDIO_ENEMY_HIT );
            break;
        }
    }
}


void Entity::SetWorldPosition( const Vec2& worldPosition ) {
    m_transform.position = worldPosition;
}


// CHEAT: Used by gameState to make player invulnerable
void Entity::SetKillable( bool isKillable ) {
    m_canDie = isKillable;
}


void Entity::Die() {
    m_isDead = true;
    m_isGarbage = true;
}


void Entity::TakeDamage( float damageToTake ) {
    g_theAudio->PlaySound( m_hitSound );

    if( m_statsManager != nullptr ) {
        m_statsManager->TakeDamage( damageToTake );
    }
}


Vec2 Entity::GetForwardVector() const {
    return Vec2( CosDegrees( m_transform.rotationDegrees ), SinDegrees( m_transform.rotationDegrees ) );
}


void Entity::BuildMesh( const Rgba& tint /*= Rgba::WHITE*/ ) {
    CPUMesh builder;
    builder.SetColor( tint );
    builder.AddQuad( m_localBounds, m_spriteUVs );

    CLEAR_POINTER( m_mesh );
    m_mesh = new GPUMesh( g_theRenderer );
    m_mesh->CopyVertsFromCPUMesh( &builder );
}


int Entity::s_numEntities = 0;
