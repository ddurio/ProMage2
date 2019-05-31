#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/Camera.hpp"


class TopDownFollowCamera : public Camera {
    public:
    TopDownFollowCamera( const Entity* targetToFollow );

    void Startup();
    void Shutdown();

    void Update( float deltaSeconds );

    void SetFollowTarget( const Entity* targetToFollow );

    private:
    const Entity* m_target = nullptr;
    Vec2 m_position = Vec2::ZERO;
};
