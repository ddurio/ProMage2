#include "Engine/Physics/RigidBodyChild2D.hpp"

#include "Engine/Debug/Profiler.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/Transform2D.hpp"
#include "Engine/Physics/Manifold.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Physics/RigidBody2D.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"


void RigidBodyChild2D::UpdateBitFields( PhysicsSystem* physSystem ) {
    Vec2 bitFieldScale = physSystem->GetBitFieldScale();
    Vec2 bitFieldOffset = physSystem->GetBitFieldScaledOffset();

    // Make world bounds
    float radius = 0.f;
    OBB2 worldBounds = GetWorldBounds( radius );
    AABB2 worldBox = worldBounds.GetBoundingAABB( radius );

    // Scale to bitfield bounds
    worldBox.mins.x *= bitFieldScale.x;
    worldBox.mins.y *= bitFieldScale.y;

    worldBox.maxs.x *= bitFieldScale.x;
    worldBox.maxs.y *= bitFieldScale.y;

    // Translate origin
    worldBox.Translate( bitFieldOffset );

    // Set bits
    int minX = Max( (int)worldBox.mins.x, 0 );
    int maxX = Min( (int)worldBox.maxs.x, 7 );

    int minY = Max( (int)worldBox.mins.y, 0 );
    int maxY = Min( (int)worldBox.maxs.y, 7 );

    m_bitFields = 0x0;

    for( int yIndex = minY; yIndex <= maxY; yIndex++ ) {
        for( int xIndex = minX; xIndex <= maxX; xIndex++ ) {
            int fieldIndex = (8 * yIndex) + xIndex;
            m_bitFields |= (1ULL << fieldIndex);
        }
    }
}


void RigidBodyChild2D::RenderDebug() const {
    m_renderContext->BindTexture();
    m_renderContext->DrawMesh( m_mesh, GetModelMatrix() );
}


void RigidBodyChild2D::SetDebugColor( const Rgba& color ) {
    if( color != m_debugColor ) {
        m_debugColor = color;
        BuildMesh();
    }
}


void RigidBodyChild2D::SetCallbackEvent( RBChildEvent event, const std::string& callbackEvent ) {
    if( event >= 0 && event < NUM_RBCHILD_EVENTS ) {
        m_callbackEvents[event] = callbackEvent;
    }
}


void RigidBodyChild2D::SetCallbackArguments( const EventArgs& args ) {
    m_callbackArgs = args; // Does not overwrite existing args
}


RBChildShape2D RigidBodyChild2D::GetShape() const {
    return m_shape;
}


OBB2 RigidBodyChild2D::GetLocalBounds( float& radiusOut ) const {
    radiusOut = m_radius;
    return m_localBounds;
}


OBB2 RigidBodyChild2D::GetWorldBounds( float& radiusOut ) const {
    radiusOut = m_radius;
    return m_localBounds.GetTransformed( m_parentTransform );
}


Matrix44 RigidBodyChild2D::GetModelMatrix() const {
    Matrix44 model = Matrix44::MakeZRotationDegrees( m_parentTransform.rotationDegrees );
    model.SetTranslation( m_parentTransform.position );

    return model;
}


RigidBody2D* RigidBodyChild2D::GetParent() const {
    return m_parent;
}


const EventArgs& RigidBodyChild2D::GetCallbackArgumens() const {
    return m_callbackArgs;
}


RigidBodyChild2D::RigidBodyChild2D( RenderContext* renderContext, RigidBody2D* parent, const AABB2& box, float radius /*= 0.f*/, const Rgba& debugColor /*= Rgba::WHITE */ ) :
    m_renderContext( renderContext ),
    m_parent( parent ),
    m_parentTransform( parent->GetTransform() ),
    m_radius( radius ),
    m_debugColor( debugColor ) {
    bool isStatic = (m_parent->GetSimulationMode() == SIMULATION_MODE_STATIC);
    bool rotationIsFixed = m_parent->GetRotationConstraint();
    m_shape = (isStatic || rotationIsFixed) ? RBCHILD_SHAPE_AABB2 : RBCHILD_SHAPE_OBB2;

    m_localBounds = OBB2( box );
    BuildMesh();
}


RigidBodyChild2D::RigidBodyChild2D( RenderContext* renderContext, RigidBody2D* parent, const OBB2& box, float radius /*= 0.f*/, const Rgba& debugColor /*= Rgba::WHITE */ ) :
    m_renderContext( renderContext ),
    m_parent( parent ),
    m_parentTransform( parent->GetTransform() ),
    m_radius( radius ),
    m_debugColor( debugColor ) {
    m_shape = RBCHILD_SHAPE_OBB2;

    m_localBounds = box;
    BuildMesh();
}


RigidBodyChild2D::RigidBodyChild2D( RenderContext* renderContext, RigidBody2D* parent, const Capsule2& capsule, const Rgba& debugColor /*= Rgba::WHITE */ ) :
    m_renderContext( renderContext ),
    m_parent( parent ),
    m_parentTransform( parent->GetTransform() ),
    m_radius( capsule.radius ),
    m_debugColor( debugColor ) {
    m_shape = RBCHILD_SHAPE_CAPSULE2;

    Vec2 right = capsule.end - capsule.start;
    Vec2 dimensions = Vec2( 0.5f * right.GetLength(), 0.f );
    m_localBounds = OBB2( capsule.GetCenter(), dimensions, right );
    BuildMesh();
}


RigidBodyChild2D::RigidBodyChild2D( RenderContext* renderContext, RigidBody2D* parent, const Vec2& discCenter, float discRadius, const Rgba& debugColor /*= Rgba::WHITE */ ) :
    m_renderContext( renderContext ),
    m_parent( parent ),
    m_parentTransform( parent->GetTransform() ),
    m_radius( discRadius ),
    m_debugColor( debugColor ) {
    m_shape = RBCHILD_SHAPE_DISC;

    m_localBounds = OBB2( discCenter, Vec2::ZERO, Vec2::RIGHT );
    BuildMesh();
}


RigidBodyChild2D::~RigidBodyChild2D() {
    CLEAR_POINTER( m_mesh );
}


bool RigidBodyChild2D::GetManifold( const RigidBodyChild2D* otherRBChild, Manifold2& out_manifold ) const {
    PROFILE_FUNCTION();

    float myRadius = m_radius;
    OBB2 myWorldBounds = GetWorldBounds( myRadius );

    if( m_shape == RBCHILD_SHAPE_DISC ) {
        Vec2 discCenterA = myWorldBounds.center;
        float discRadiusA = myRadius;
    
        float otherRadius = otherRBChild->m_radius;
        OBB2 otherWorldBounds = otherRBChild->GetWorldBounds( otherRadius );

        if( otherRBChild->m_shape == RBCHILD_SHAPE_DISC ) {            // Disc v Disc
            return Manifold2::GetManifold( discCenterA, discRadiusA, otherWorldBounds.center, otherRadius, out_manifold );

        } else if( otherRBChild->m_shape == RBCHILD_SHAPE_AABB2 ) {    // Disc v padded AABB2
            Vec2 corners[4];
            otherWorldBounds.GetCorners( corners );
            AABB2 otherWorldAABB = AABB2( corners[2], corners[1] );

            return Manifold2::GetManifold( discCenterA, discRadiusA, otherWorldAABB, otherRadius, out_manifold );

        } else {                                                    // Disc v padded OBB2
            return Manifold2::GetManifold( discCenterA, discRadiusA, otherWorldBounds, otherRadius, out_manifold );
        }

    } else if( m_shape == RBCHILD_SHAPE_AABB2 ) {
        Vec2 corners[4];
        myWorldBounds.GetCorners( corners );
        AABB2 myWorldAABB = AABB2( corners[2], corners[1] );

        float otherRadius = otherRBChild->m_radius;
        OBB2 otherWorldBounds = otherRBChild->GetWorldBounds( otherRadius );

        if( otherRBChild->m_shape == RBCHILD_SHAPE_DISC ) {            // padded AABB2 v Disc
            return Manifold2::GetManifold( myWorldAABB, myRadius, otherWorldBounds.center, otherRadius, out_manifold );
        
        } else if( otherRBChild->m_shape == RBCHILD_SHAPE_AABB2 &&     // AABB2 v AABB2 (no padding)
                   myRadius == 0.f && otherRadius == 0.f )
        {
            otherWorldBounds.GetCorners( corners );
            AABB2 otherWorldAABB = AABB2( corners[2], corners[1] );

            return Manifold2::GetManifold( myWorldAABB, otherWorldAABB, out_manifold );

        } else {                                                    // padded OBB2 v padded OBB2
            return Manifold2::GetManifold( myWorldBounds, myRadius, otherWorldBounds, otherRadius, out_manifold );
        }

    } else {
        float otherRadius = otherRBChild->m_radius;
        OBB2 otherWorldBounds = otherRBChild->GetWorldBounds( otherRadius );

        return Manifold2::GetManifold( myWorldBounds, myRadius, otherWorldBounds, otherRadius, out_manifold );
    }
}


void RigidBodyChild2D::BuildMesh() {
    if( m_mesh != nullptr ) {
        delete m_mesh;
        m_mesh = nullptr;
    }

    CPUMesh builder;
    builder.SetColor( m_debugColor );
    float lineThickness = 0.05f;

    switch( m_shape ) {
        case( RBCHILD_SHAPE_CAPSULE2 ): {
            BuildCapsule( builder, lineThickness );
            break;
        } case( RBCHILD_SHAPE_DISC ): {
            BuildDisc( builder, lineThickness );
            break;
        } case( RBCHILD_SHAPE_AABB2 ): { // AABB is the same as OBB, intentional fall-through
        } case( RBCHILD_SHAPE_OBB2 ): { // Defaults to OBB, intentional fall-through
        } default: {
            builder.AddQuadEdge( m_localBounds, lineThickness );
            break;
        }
    }

    m_mesh = new GPUMesh( m_renderContext );
    m_mesh->CopyVertsFromCPUMesh( &builder );
}


void RigidBodyChild2D::BuildCapsule( CPUMesh& builder, float lineThickness ) {
    Vec2 corners[4];
    m_localBounds.GetCorners( corners[0], corners[1], corners[2], corners[3] );
    const Vec2& start = corners[0];
    const Vec2& end = corners[1];

    Capsule2 capsule = Capsule2( start, end, m_radius );
    builder.AddCapsuleEdge( capsule, lineThickness );
}


void RigidBodyChild2D::BuildDisc( CPUMesh& builder, float lineThickness ) {
    builder.AddRing( m_localBounds.center, m_radius - lineThickness, m_radius );
    builder.AddLine( m_localBounds.center, m_localBounds.center + Vec2( m_radius - lineThickness, 0.f ), lineThickness );
}


void RigidBodyChild2D::Destroy() {
    m_isGarbage = true;
}


