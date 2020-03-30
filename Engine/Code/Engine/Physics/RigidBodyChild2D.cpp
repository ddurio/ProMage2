#include "Engine/Physics/RigidBodyChild2D.hpp"

#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/Transform2D.hpp"
#include "Engine/Physics/RigidBody2D.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"


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


void* RigidBodyChild2D::GetGameObject() const {
    return m_parent->GetGameObject();
}


RigidBodyChild2D::RigidBodyChild2D( RenderContext* renderContext, RigidBody2D* parent, const AABB2& box, float radius /*= 0.f*/, const Rgba& debugColor /*= Rgba::WHITE */ ) :
    m_renderContext( renderContext ),
    m_parent( parent ),
    m_parentTransform( parent->GetTransform() ),
    m_radius( radius ),
    m_debugColor( debugColor ) {
    m_shape = RBCHILD_SHAPE_AABB2;

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
            BuildBox( builder, lineThickness );
            break;
        }
    }

    m_mesh = new GPUMesh( m_renderContext );
    m_mesh->CopyVertsFromCPUMesh( &builder );
}


void RigidBodyChild2D::BuildCapsule( CPUMesh& builder, float lineThickness ) {
    Vec2 corners[4];
    m_localBounds.GetCorners( corners[0], corners[1], corners[2], corners[3] );
    Vec2& start = corners[0];
    Vec2& end = corners[1];

    if( start == end ) {
        builder.AddRing( m_localBounds.center, m_radius - lineThickness, m_radius );
        builder.AddLine( m_localBounds.center, m_localBounds.center + Vec2( m_radius - lineThickness, 0.f ), lineThickness );
    } else {
        Capsule2 capsule = Capsule2( start, end, m_radius );
        builder.AddCapsuleEdge( capsule, lineThickness );
    }
}


void RigidBodyChild2D::BuildDisc( CPUMesh& builder, float lineThickness ) {
    builder.AddRing( m_localBounds.center, m_radius - lineThickness, m_radius );
    builder.AddLine( m_localBounds.center, m_localBounds.center + Vec2( m_radius, 0.f ), lineThickness );
}


void RigidBodyChild2D::BuildBox( CPUMesh& builder, float lineThickness ) {
    Vec2 botLeft;
    Vec2 botRight;
    Vec2 topLeft;
    Vec2 topRight;
    m_localBounds.GetCorners( topLeft, topRight, botLeft, botRight );

    builder.AddLine( topLeft, topRight, lineThickness );
    builder.AddLine( topRight, botRight, lineThickness );
    builder.AddLine( botRight, botLeft, lineThickness );
    builder.AddLine( botLeft, topLeft, lineThickness );
}


void RigidBodyChild2D::Destroy() {
    m_isGarbage = true;
}


