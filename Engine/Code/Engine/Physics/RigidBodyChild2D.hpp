#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/OBB2.hpp"


enum RBChildShape2D {
    RBCHILD_SHAPE_INVALID2 = -1,

    RBCHILD_SHAPE_AABB2,
    RBCHILD_SHAPE_DISC,
    RBCHILD_SHAPE_POINT2,
    RBCHILD_SHAPE_CAPSULE2,
    RBCHILD_SHAPE_LINE2,
    RBCHILD_SHAPE_OBB2,

    NUM_RBCHILD_SHAPES
};


enum RBChildEvent {
    RBCHILD_EVENT_COLLIDE,
    RBCHILD_EVENT_ENTER,
    RBCHILD_EVENT_LEAVE,

    NUM_RBCHILD_EVENTS
};


const std::string PHYSICS_ARG_MY_COLLIDER       = "myCollider";
const std::string PHYSICS_ARG_MY_TRIGGER        = "myTrigger";
const std::string PHYSICS_ARG_OTHER_COLLIDER    = "otherCollider";
const std::string PHYSICS_ARG_MANIFOLD          = "manifold";
const std::string PHYSICS_ARG_FIRST_FRAME       = "firstFrame";
const std::string PHYSICS_ARG_LATEST_FRAME      = "latestFrame";


class CPUMesh;
class GPUMesh;
class RenderContext;
class RigidBody2D;

struct Capsule2;
struct Matrix44;


class RigidBodyChild2D {
    public:
    void RenderDebug() const;
    void SetDebugColor( const Rgba& color );

    void SetCallbackEvent( RBChildEvent event, const std::string& callbackEvent );

    RBChildShape2D GetShape() const;
    OBB2 GetLocalBounds( float& radiusOut ) const;
    OBB2 GetWorldBounds( float& radiusOut ) const;
    Matrix44 GetModelMatrix() const;

    RigidBody2D* GetParent() const;
    void* GetGameObject() const;

    protected:
    RigidBodyChild2D( RenderContext* renderContext, RigidBody2D* parent, const AABB2& box, float radius = 0.f, const Rgba& debugColor = Rgba::WHITE );
    RigidBodyChild2D( RenderContext* renderContext, RigidBody2D* parent, const OBB2& box, float radius = 0.f, const Rgba& debugColor = Rgba::WHITE );
    RigidBodyChild2D( RenderContext* renderContext, RigidBody2D* parent, const Capsule2& capsule, const Rgba& debugColor = Rgba::WHITE );
    RigidBodyChild2D( RenderContext* renderContext, RigidBody2D* parent, const Vec2& discCenter, float discRadius, const Rgba& debugColor = Rgba::WHITE );

    virtual ~RigidBodyChild2D();


    RenderContext* m_renderContext = nullptr;
    RigidBody2D* m_parent = nullptr;
    const Transform2D& m_parentTransform;

    GPUMesh* m_mesh = nullptr;
    RBChildShape2D m_shape = RBCHILD_SHAPE_INVALID2;
    OBB2 m_localBounds = OBB2();
    float m_radius = 0.f;
    Rgba m_debugColor = Rgba::WHITE;

    bool m_isGarbage = false;

    std::string m_callbackEvents[NUM_RBCHILD_EVENTS];


    virtual void FireCallbackEvent( RBChildEvent event, void* info ) = 0;

    void BuildMesh();
    void BuildCapsule( CPUMesh& builder, float lineThickness );
    void BuildDisc( CPUMesh& builder, float lineThickness );
    void BuildBox( CPUMesh& builder, float lineThickness );
    virtual void Destroy();
};
