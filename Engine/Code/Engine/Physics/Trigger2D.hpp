#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Physics/Manifold.hpp"
#include "Engine/Physics/RigidBodyChild2D.hpp"


class RenderContext;
class RigidBody2D;

struct Capsule2;


class Collider2D;;
class Trigger2D;

struct TriggerInfo2D {
    TriggerInfo2D() {};
    explicit TriggerInfo2D( const Trigger2D* myTriggerIn, const Collider2D* otherColliderIn, int firstFrameIn, int latestFrameIn );

    const Trigger2D* myTrigger = nullptr;
    const Collider2D* otherCollider = nullptr;
    int firstFrame = -1;
    int latestFrame = -1;
};


class Trigger2D : public RigidBodyChild2D {
    friend class RigidBody2D;
    friend class Collider2D; // Needs to call fireCallbackEvent on death

    public:
    bool GetTriggerInfo( Collider2D* colliderB, bool isImpossibleToOverlap, TriggerInfo2D& outCollisionInfo );
    TriggerInfo2D GetExistingTriggerInfo( const Collider2D* collider ) const;


    private:
    std::map< const Collider2D*, TriggerInfo2D > m_triggerInfos;


    Trigger2D( RenderContext* renderContext, RigidBody2D* parent, const AABB2& box, float radius = 0.f, const Rgba& debugColor = Rgba::WHITE );
    Trigger2D( RenderContext* renderContext, RigidBody2D* parent, const OBB2& box, float radius = 0.f, const Rgba& debugColor = Rgba::WHITE );
    Trigger2D( RenderContext* renderContext, RigidBody2D* parent, const Capsule2& capsule, const Rgba& debugColor = Rgba::WHITE );
    Trigger2D( RenderContext* renderContext, RigidBody2D* parent, const Vec2& discCenter, float discRadius, const Rgba& debugColor = Rgba::WHITE );
    ~Trigger2D() {};

    void FireCallbackEvent( RBChildEvent event, void* info ) override;
};
