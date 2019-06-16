#include "Game/Entity.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/Capsule3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/Game.hpp"
#include "Game/Map.hpp"


class RigidBody2D;


Entity::Entity( Map* theMap, EntityType type /*= ENTITY_TYPE_UNKNOWN*/, FactionID faction /*= FACTION_UNKNOWN */ ) :
    m_map( theMap ),
    m_entityType( type ) {
    SetFaction( faction );
}


Entity::~Entity() {
    CLEAR_POINTER( m_mesh );

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
    return m_isKillable;
}


bool Entity::IsSolid() const {
    return m_isSolid;
}


bool Entity::IsMovable() const {
    return m_isMovable;
}


int Entity::GetHealth() const {
    return m_health;
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


void Entity::Die() {
    m_isDead = true;
    m_isGarbage = true;
}


void Entity::TakeDamage( int damageToTake ) {
    g_theAudio->PlaySound( m_hitSound );
    m_health -= damageToTake;

    if( m_health <= 0 ) {
        Die();
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
