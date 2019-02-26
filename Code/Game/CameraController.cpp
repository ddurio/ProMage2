#include "Game/CameraController.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Renderer/Camera.hpp"

#include "Game/Game.hpp"


CameraController::CameraController( Camera* camera ) :
    m_camera(camera) {
}


void CameraController::Startup() {
    g_theWindow->SetMouseMode( MOUSE_MODE_RELATIVE );
    g_theWindow->LockMouse();
    g_theWindow->HideMouse();
}


void CameraController::Shutdown() {

}


void CameraController::Update( float deltaSeconds ) {
    IntVec2 mouseMovement = g_theWindow->GetMouseClientDisplacement();
    Vec3 mouseRotation = Vec3( (float)mouseMovement.y / 50.f, (float)mouseMovement.x / 50.f, 0.f );
    m_rotation += mouseRotation;

    int leftRight = -((int)m_leftPressed) + (int)m_rightPressed;
    int backForward = -((int)m_backPressed) + (int)m_forwardPressed;

    float moveX = leftRight * m_moveSpeed * deltaSeconds;
    float moveZ = backForward * m_moveSpeed * deltaSeconds;

    Matrix44 model = Matrix44::MakeRotationDegrees3D( m_rotation );
    Vec3 localMovement = Vec3( moveX, 0.f, moveZ );
    Vec3 worldMovement = model.TransformPosition3D( localMovement );
    m_position += worldMovement;

    model.SetTranslation( m_position );
    m_camera->SetModelMatrix( model );
}


void CameraController::Render() const {

}


void CameraController::Die() {

}


bool CameraController::HandleKeyPressed( unsigned char keyCode ) {
    switch( keyCode ) {
        case('W'): { // Up
            m_forwardPressed = true;
            return true;
        } case('A'): { // Left
            m_leftPressed = true;
            return true;
        } case('S'): { // Down
            m_backPressed = true;
            return true;
        } case('D'): { // Right
            m_rightPressed = true;
            return true;
        }
    }

    return false;
}


bool CameraController::HandleKeyReleased( unsigned char keyCode ) {
    switch( keyCode ) {
        case('W'): { // Up
            m_forwardPressed = false;
            return true;
        } case('A'): { // Left
            m_leftPressed = false;
            return true;
        } case('S'): { // Down
            m_backPressed = false;
            return true;
        } case('D'): { // Right
            m_rightPressed = false;
            return true;
        }
    }

    return false;
}

