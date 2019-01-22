#include "Game/Entity.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"


Entity::Entity( Game* theGame ) {
	m_theGame = theGame;
}


const Vec2 Entity::GetPosition() const {
    return m_position;
}


bool Entity::IsAlive() const {
    return !m_isDead;
}


bool Entity::IsGarbage() const {
    return m_isGarbage;
}


void Entity::GetPhysicsDisc( Vec2& position, float& radius ) const {
    position = m_position;
    radius = m_physicsRadius;
}


void Entity::GetCosmeticDist( Vec2& position, float& radius ) const {
    position = m_position;
    radius = m_cosmeticRadius;
}


void Entity::TakeDamage( int damageToTake ) {
    m_health -= damageToTake;

    if( m_health <= 0 ) {
        Die();
    }
}


Vec2 Entity::GetForwardVector() const {
    return Vec2( CosDegrees( m_orientationDegrees ), SinDegrees( m_orientationDegrees ) );
}


void Entity::UpdateDebugVerts() {
    Vec3 triOuterVert = Vec3( 1, 0, 0 );
    float degreesPerTriangle = 360 / (s_numDebugVerts / 3);
    float vertAngle;
    int numTriangle;
    
    for( int i = 0; i < s_numDebugVerts; i += 3 ) {
        numTriangle = i / 3;
        // Triangle Vert1
        m_debugCosmeticVerts[i].m_position = Vec3( m_position.x, m_position.y, 0.f );
        m_debugCosmeticVerts[i].m_color = m_debugCosmeticColor;

        // Triangle Vert2
        vertAngle = numTriangle * degreesPerTriangle;
        m_debugCosmeticVerts[i + 1].m_position = TransformPosition( Vec3( 1, 0, 0 ), m_cosmeticRadius, vertAngle, m_position );
        m_debugCosmeticVerts[i + 1].m_color = m_debugCosmeticColor;

        // Triangle Vert3
        vertAngle = (numTriangle + 1) * degreesPerTriangle;
        m_debugCosmeticVerts[i + 2].m_position = TransformPosition( Vec3( 1, 0, 0 ), m_cosmeticRadius, vertAngle, m_position );
        m_debugCosmeticVerts[i + 2].m_color = m_debugCosmeticColor;
    }
    m_debugCosmeticVerts[s_numDebugVerts - 1] = m_debugCosmeticVerts[1];

    for( int i = 0; i < s_numDebugVerts; i += 3 ) {
        numTriangle = i / 3;
        // Triangle Vert1
        m_debugPhysicsVerts[i].m_position = Vec3( m_position.x, m_position.y, 0.f );
        //m_debugPhysicsVerts[i].m_position = Vec3( 0, 0, 0 );
        m_debugPhysicsVerts[i].m_color = m_debugPhysicsColor;

        // Triangle Vert2
        vertAngle = numTriangle * degreesPerTriangle;
        m_debugPhysicsVerts[i + 1].m_position = TransformPosition( Vec3( 1, 0, 0 ), m_physicsRadius, vertAngle, m_position );
        m_debugPhysicsVerts[i + 1].m_color = m_debugPhysicsColor;

        // Triangle Vert3
        vertAngle = (numTriangle + 1) * degreesPerTriangle;
        m_debugPhysicsVerts[i + 2].m_position = TransformPosition( Vec3( 1, 0, 0 ), m_physicsRadius, vertAngle, m_position );
        m_debugPhysicsVerts[i + 2].m_color = m_debugPhysicsColor;
    }
    m_debugPhysicsVerts[s_numDebugVerts - 1] = m_debugPhysicsVerts[1];
}
