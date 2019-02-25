#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"


class Entity {
	public:
    Entity() {};

	virtual void Startup() = 0;
	virtual void Shutdown() = 0;

	virtual void Update( float deltaSeconds ) = 0;
	virtual void Render() const = 0;
    virtual void Die() = 0;

    bool IsAlive() const;
    bool IsGarbage() const;
    const Vec2 GetPosition() const;
    void GetPhysicsDisc( Vec2& position, float& radius) const;
    void GetCosmeticDist( Vec2& position, float& radius) const;
    void TakeDamage( int damageToTake );

	protected:
	Vec2 m_position;
	Vec2 m_velocity;
    float m_angularVelocity = 0;
    float m_orientationDegrees = 0;
	float m_physicsRadius;
	float m_cosmeticRadius;
	int m_health = 1;

    bool m_isDead = false;
    bool m_isGarbage = false;

    const Rgba m_debugCosmeticColor = Rgba( 1, 0, 1, 1 );
    const Rgba m_debugPhysicsColor = Rgba( 0, 1, 1, 1 );
    static const int s_numDebugVerts = 63;

    Vertex_PCU m_debugCosmeticVerts[s_numDebugVerts];
    Vertex_PCU m_debugPhysicsVerts[s_numDebugVerts];


    Vec2 GetForwardVector() const;
    void UpdateDebugVerts();
};
