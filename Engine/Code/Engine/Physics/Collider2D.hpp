#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Physics/Manifold.hpp"
#include "Engine/Physics/RigidBodyChild2D.hpp"


class RenderContext;
class RigidBody2D;
class Trigger2D;

struct Capsule2;


class Collider2D;
struct CollisionInfo2D {
    CollisionInfo2D() {};
    explicit CollisionInfo2D( const Collider2D* myColliderIn, const Collider2D* otherColliderIn, const Manifold2& manifoldIn );
    CollisionInfo2D GetInverse() const;

    const Collider2D* myCollider = nullptr;
    const Collider2D* otherCollider = nullptr;
    Manifold2 manifold;
};


class Collider2D : public RigidBodyChild2D {
    friend class RigidBody2D;

    public:
    void SetIsColliding( bool isColliding );
    float SetMomentOfInertia( float colliderMass );

    bool IsColliding( const Collider2D* otherCollider ) const;
    bool GetIsColliding() const;

    float GetArea() const;
    float GetMomentOfInertia() const;

    bool GetCollisionInfo( const Collider2D* colliderB, CollisionInfo2D& outCollisionInfo ) const;


    private:
    float m_moment = 0.f;
    bool m_isColliding = false;
    std::vector<Trigger2D*> m_overlappingTriggers;


    Collider2D( RenderContext* renderContext, RigidBody2D* parent, const AABB2& box, float radius = 0.f, const Rgba& debugColor = Rgba::WHITE );
    Collider2D( RenderContext* renderContext, RigidBody2D* parent, const OBB2& box, float radius = 0.f, const Rgba& debugColor = Rgba::WHITE );
    Collider2D( RenderContext* renderContext, RigidBody2D* parent, const Capsule2& capsule, const Rgba& debugColor = Rgba::WHITE );
    Collider2D( RenderContext* renderContext, RigidBody2D* parent, const Vec2& discCenter, float discRadius, const Rgba& debugColor = Rgba::WHITE );
    ~Collider2D() {};

    void FireCallbackEvent( RBChildEvent event, void* info ) override;
    void Destroy() override;
};