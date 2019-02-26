#pragma once
#include "Game/GameCommon.hpp"

#include "Game/Entity.hpp"

class Camera;


class CameraController : public Entity {
    public:
    CameraController( Camera* camera );

    void Startup() override;
    void Shutdown() override;

    void Update( float deltaSeconds ) override;
    void Render() const override;
    void Die() override;

    bool HandleKeyPressed( unsigned char keyCode );
    bool HandleKeyReleased( unsigned char keyCode );

    private:
    Camera* m_camera = nullptr;
    Vec3 m_position = Vec3(0.f, 0.f, -10.f);
    Vec3 m_rotation = Vec3::ZERO;

    bool m_leftPressed = false;
    bool m_rightPressed = false;

    bool m_forwardPressed = false;
    bool m_backPressed = false;

    float m_moveSpeed = 2.f;
};
