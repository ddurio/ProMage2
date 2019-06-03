#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/Capsule3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Physics/RigidBody2D.hpp"

#include "vector"


enum EntityType {
    ENTITY_TYPE_UNKNOWN = -1,

    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_ENEMY,
    ENTITY_TYPE_PROJECTILE,
    ENTITY_TYPE_PICKUP,
    ENTITY_TYPE_ACTOR,

    NUM_ENTITY_TYPES
};

enum FactionID {
    FACTION_UNKNOWN = -1,
    FACTION_PLAYER0,
    FACTION_PLAYER1,
    FACTION_PLAYER2,
    FACTION_PLAYER3,

    FACTION_ENEMY1,

    NUM_FACTIONS
};


class GPUMesh;
class Map;
class Material;
class Texture;


class Entity {
    friend class Map;

    public:
    explicit Entity( Map* theMap, EntityType type = ENTITY_TYPE_UNKNOWN, FactionID faction = FACTION_UNKNOWN );
    virtual ~Entity();

    virtual void Startup() = 0;
    virtual void Shutdown() = 0;

    virtual void Update( float deltaSeconds ) = 0;
    virtual void Render() const = 0;

    virtual void Die();
    void TakeDamage( int damageToTake );

    bool IsAlive() const;
    virtual bool IsGarbage() const;
    bool IsKillable() const;
    bool IsSolid() const;
    bool IsMovable() const;

    const Vec2 GetPosition() const;
    const Transform2D* GetTransformPointer() const;

    const FactionID GetFaction() const;
    const EntityType GetEntityType() const;
    RigidBody2D* GetRigidBody() const;
    int GetHealth() const;

    void SetFaction( FactionID faction );
    void SetWorldPosition( const Vec2& worldPosition );

    protected:
    Map* m_map = nullptr;
    const EntityType m_entityType = ENTITY_TYPE_UNKNOWN;
    FactionID m_faction = FACTION_UNKNOWN;
    Rgba m_factionTint = Rgba::WHITE;

    SoundID m_hitSound = MISSING_SOUND_ID;

    Transform2D m_transform;
    Vec2 m_velocity;
    RigidBody2D* m_rigidBody = nullptr;

    float m_angularVelocity = 0;

    float m_physicsRadius = 0.f;
    float m_cosmeticRadius = 0.f;
    float m_scale = 1.f;

    int m_health = 1;
    bool m_isKillable = true;
    bool m_isSolid = true;
    bool m_isMovable = true;

    bool m_canSee = true;
    bool m_canWalk = true;
    bool m_canFly = false;
    bool m_canSwim = true;

    bool m_isDead = false;
    bool m_isGarbage = false;

    const Rgba m_debugCosmeticColor = Rgba::MAGENTA;
    const Rgba m_debugPhysicsColor = Rgba::CYAN;

    GPUMesh* m_mesh = nullptr;
    Material* m_material = nullptr;

    OBB2 m_localBounds = OBB2();
    AABB2 m_spriteUVs = AABB2::ZEROTOONE;

    Vec2 GetForwardVector() const;
    virtual void BuildMesh( const Rgba& tint = Rgba::WHITE );
};
